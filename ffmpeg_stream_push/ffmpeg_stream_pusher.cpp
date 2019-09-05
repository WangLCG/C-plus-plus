#include "ffmpeg_stream_pusher.h"
void bs_init(bs_t *s, void *p_data, int i_data)
{
    s->p_start = (unsigned char *)p_data;       //用传入的p_data首地址初始化p_start，只记下有效数据的首地址
    s->p = (unsigned char *)p_data;       //字节首地址，一开始用p_data初始化，每读完一个整字节，就移动到下一字节首地址
    s->p_end = s->p + i_data;                 //尾地址，最后一个字节的首地址?
    s->i_left = 8;                             //还没有开始读写，当前字节剩余未读取的位是8
}

int bs_read(bs_t *s, int i_count)
{
    static unsigned int i_mask[33] = { 0x00,
        0x01, 0x03, 0x07, 0x0f,
        0x1f, 0x3f, 0x7f, 0xff,
        0x1ff, 0x3ff, 0x7ff, 0xfff,
        0x1fff, 0x3fff, 0x7fff, 0xffff,
        0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
        0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff };

        /*
    数组中的元素用二进制表示如下：

    假设：初始为0，已写入为+，已读取为-

    字节:       1       2       3       4
    00000000 00000000 00000000 00000000     下标
    0x00:                             00000000      x[0]

    0x01:                             00000001      x[1]
    0x03:                             00000011      x[2]
    0x07:                             00000111      x[3]
    0x0f:                             00001111      x[4]

    0x1f:                             00011111      x[5]
    0x3f:                             00111111      x[6]
    0x7f:                             01111111      x[7]
    0xff:                             11111111      x[8]    1字节

    0x1ff:                       0001 11111111      x[9]
    0x3ff:                       0011 11111111      x[10]   i_mask[s->i_left]
    0x7ff:                       0111 11111111      x[11]
    0xfff:                       1111 11111111      x[12]   1.5字节

    0x1fff:                  00011111 11111111      x[13]
    0x3fff:                  00111111 11111111      x[14]
    0x7fff:                  01111111 11111111      x[15]
    0xffff:                  11111111 11111111      x[16]   2字节

    0x1ffff:                0001 11111111 11111111      x[17]
    0x3ffff:                0011 11111111 11111111      x[18]
    0x7ffff:                0111 11111111 11111111      x[19]
    0xfffff:                1111 11111111 11111111      x[20]   2.5字节

    0x1fffff:           00011111 11111111 11111111      x[21]
    0x3fffff:           00111111 11111111 11111111      x[22]
    0x7fffff:           01111111 11111111 11111111      x[23]
    0xffffff:           11111111 11111111 11111111      x[24]   3字节

    0x1ffffff:     0001 11111111 11111111 11111111      x[25]
    0x3ffffff:     0011 11111111 11111111 11111111      x[26]
    0x7ffffff:    0111 11111111 11111111 11111111       x[27]
    0xfffffff:    1111 11111111 11111111 11111111       x[28]   3.5字节

    0x1fffffff:00011111 11111111 11111111 11111111      x[29]
    0x3fffffff:00111111 11111111 11111111 11111111      x[30]
    0x7fffffff:01111111 11111111 11111111 11111111      x[31]
    0xffffffff:11111111 11111111 11111111 11111111      x[32]   4字节

    */

    int      i_shr;             //
    int i_result = 0;           //用来存放读取到的的结果 typedef unsigned   uint32_t;

    while (i_count > 0)        //要读取的比特数
    {
        if (s->p >= s->p_end)  //字节流的当前位置>=流结尾，即代表此比特流s已经读完了。
        {                       //
            break;
        }

        if ((i_shr = s->i_left - i_count) >= 0)  //当前字节剩余的未读位数，比要读取的位数多，或者相等
        {
            //i_left当前字节剩余的未读位数，本次要读i_count比特，i_shr=i_left-i_count的结果如果>=0，说明要读取的都在当前字节内
            //i_shr>=0，说明要读取的比特都处于当前字节内
            //这个阶段，一次性就读完了，然后返回i_result(退出了函数)
            /* more in the buffer than requested */
            i_result |= (*s->p >> i_shr)&i_mask[i_count];//“|=”:按位或赋值，A |= B 即 A = A|B
            //|=应该在最后执行，把结果放在i_result(按位与优先级高于复合操作符|=)
            //i_mask[i_count]最右侧各位都是1,与括号中的按位与，可以把括号中的结果复制过来
            //!=,左边的i_result在这儿全是0，右侧与它按位或，还是复制结果过来了，好象好几步都多余
            /* 读取后，更新结构体里的字段值 */
            s->i_left -= i_count; //即i_left = i_left - i_count，当前字节剩余的未读位数，原来的减去这次读取的
            if (s->i_left == 0)    //如果当前字节剩余的未读位数正好是0，说明当前字节读完了，就要开始下一个字节
            {
                s->p++;             //移动指针，所以p好象是以字节为步长移动指针的
                s->i_left = 8;      //新开始的这个字节来说，当前字节剩余的未读位数，就是8比特了
            }
            return(i_result);     //可能的返回值之一为：00000000 00000000 00000000 00000001 (4字节长)
        }
        else    /* i_shr < 0 ,跨字节的情况*/
        {
            //这个阶段，是while的一次循环，可能还会进入下一次循环，第一次和最后一次都可能读取的非整字节，比如第一次读了3比特，中间读取了2字节(即2x8比特)，最后一次读取了1比特，然后退出while循环
            //当前字节剩余的未读位数，比要读取的位数少，比如当前字节有3位未读过，而本次要读7位
            //???对当前字节来说，要读的比特，都在最右边，所以不再移位了(移位的目的是把要读的比特放在当前字节最右)
            /* less(较少的) in the buffer than requested */
            i_result |= (*s->p&i_mask[s->i_left]) << -i_shr;    //"-i_shr"相当于取了绝对值
            //|= 和 << 都是位操作符，优先级相同，所以从左往右顺序执行
            //举例:int|char ，其中int是4字节，char是1字节，sizeof(int|char)是4字节
            //i_left最大是8，最小是0，取值范围是[0,8]
            i_count -= s->i_left;  //待读取的比特数，等于原i_count减去i_left，i_left是当前字节未读过的比特数，而此else阶段，i_left代表的当前字节未读的比特全被读过了，所以减它
            s->p++; //定位到下一个新的字节
            s->i_left = 8;  //对一个新字节来说，未读过的位数当然是8，即本字节所有位都没读取过
        }
    }

    return(i_result);//可能的返回值之一为：00000000 00000000 00000000 00000001 (4字节长)
}

int bs_read1(bs_t *s)
{

    if (s->p < s->p_end)
    {
        unsigned int i_result;

        s->i_left--;                           //当前字节未读取的位数少了1位
        i_result = (*s->p >> s->i_left) & 0x01;//把要读的比特移到当前字节最右，然后与0x01:00000001进行逻辑与操作，因为要读的只是一个比特，这个比特不是0就是1，与0000 0001按位与就可以得知此情况
        if (s->i_left == 0)                   //如果当前字节剩余未读位数是0，即是说当前字节全读过了
        {
            s->p++;                            //指针s->p 移到下一字节
            s->i_left = 8;                     //新字节中，未读位数当然是8位
        }
        return i_result;                       //unsigned int
    }

    return 0;                                  //返回0应该是没有读到东西
}

int bs_read_ue(bs_t *s)
{
    int i = 0;
    while (bs_read1(s) == 0 && s->p < s->p_end && i < 32)    //条件为：读到的当前比特=0，指针未越界，最多只能读32比特
    {
        i++;
    }

    return((1 << i) - 1 + bs_read(s, i));
}

int H264_GetFrameType(uint8_t* bitstream, int bitstreamSize, int startCodeSize)
{
    int offset = 0;
    int length = 0;
    bs_t s;
    int frameType = H264_FRAME_UNKNOWN;

    if (!bitstream || bitstreamSize <= (startCodeSize + 1) || (startCodeSize != 3 && startCodeSize != 4))
        return H264_FRAME_UNKNOWN;

    if (startCodeSize == 4)
    {
        // 搜索有效的起始头
        offset = 0;
        for (int len = 0; len < bitstreamSize - 5; len++)
        {
            if (bitstream[len] != 0 ||
                bitstream[len + 1] != 0 ||
                bitstream[len + 2] != 0 ||
                bitstream[len + 3] != 1)
                continue;
            int naluType = bitstream[len + 4] & 0x1f;
            if (naluType == H264_NAL_SLICE || naluType == H264_NAL_SLICE_IDR)
            {
                // 搜索到起始头
                offset = len + 5;
                break;
            }
        }

        if (offset <= 0)
        {
            // 未搜索到有效的起始头
            return frameType;
        }

        // 搜索下一个起始头，计算有效的NALU长度
        length = 0;
        for (int len = offset; len < bitstreamSize - 4; len++)
        {
            if (bitstream[len] == 0 &&
                bitstream[len + 1] == 0 &&
                bitstream[len + 2] == 0 &&
                bitstream[len + 3] == 1)
                {
                length = len - offset;
                break;
            }
        }

        if (length <= 0)
        {
            // 未搜索到更多的起始头，长度为整个剩余的码流长度
            length = bitstreamSize - offset;
        }
    }
    else
    {
        // 搜索有效的起始头
        offset = 0;
        for (int len = 0; len < bitstreamSize - 4; len++)
        {
            if (bitstream[len] != 0 ||
                bitstream[len + 1] != 0 ||
                bitstream[len + 2] != 1)
                continue;
            int naluType = bitstream[len + 3] & 0x1f;
            if (naluType == H264_NAL_SLICE || naluType == H264_NAL_SLICE_IDR)
            {
                // 搜索到起始头
                offset = len + 4;
                break;
            }
        }

        if (offset <= 0)
        {
            // 未搜索到有效的起始头
            return frameType;
        }

        // 搜索下一个起始头，计算有效的NALU长度
        length = 0;
        for (int len = offset; len < bitstreamSize - 3; len++)
        {
            if (bitstream[len] == 0 &&
                bitstream[len + 1] == 0 &&
                bitstream[len + 2] == 1)
            {
                length = len - offset;
                break;
            }
        }

        if (length <= 0)
        {
            // 未搜索到更多的起始头，长度为整个剩余的码流长度
            length = bitstreamSize - offset;
        }
    }

     // 按语法解析帧类型
    bs_init(&s, (unsigned char*)bitstream + offset, length);
    bs_read_ue(&s);

    int frameFlag = bs_read_ue(&s);
    switch (frameFlag)
    {
    case 0: case 5: /* P */
        frameType = H264_FRAME_P;
        break;
    case 1: case 6: /* B */
        frameType = H264_FRAME_B;
        break;
    case 3: case 8: /* SP */
        frameType = H264_FRAME_SP;
        break;
    case 2: case 7: /* I */
        frameType = H264_FRAME_I;
        break;
    case 4: case 9: /* SI */
        frameType = H264_FRAME_SI;
        break;
    }

    return frameType;

}

/* 直接读buffer数据后填充AVPack结构体，然后推流 */
int test_pusher_rtmp_main2()
{
    int ret;
    char buf[1024] = "";
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVStream *out_stream = NULL;
    AVPacket pkt = { 0 };
    AVPacket in_pkt = { 0 };
     AVDictionary *dic = NULL;
    AVCodecContext *avctx = NULL;

    avcodec_register_all();
    av_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    avformat_network_init();

    int frame_cnt = 0;
    //Read_Data read_handle("test.h265");
    const int buffer_size = 1024 * 1024 * 3;
    uint8_t* frame_buf = new uint8_t[buffer_size];
    memset(frame_buf, 0, buffer_size);

    //const char* in_filename = "cuc_ieschool.flv";
    const char* in_filename = "test.h264";
    //const char* in_filename = "4K.h264";
    AVFormatContext *ifmt_ctx = NULL;
    int videoindex = -1;

    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("Could not open input file.");
        memset(buf, 0 ,1024);
        av_strerror(ret, buf, 1024);
        printf("Couldn't open file %s with error[%s]\n", in_filename, buf);
        return -1;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information");
        return -1;
    }

    int i = 0;
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        videoindex = i;
        break;
    }

    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    const char* out_filename = "rtmp://localhost:1936/live/livestream";
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename);

    //const char* out_filename = "rtsp://localhost:556/livestream";
    /* allocate the output media context */
    //avformat_alloc_output_context2(&ofmt_ctx, NULL, "rtsp", out_filename);
    if (!ofmt_ctx) 
    {
        printf("Could not deduce output format from file extension: using AVI.\n");
        //avformat_alloc_output_context2(&ofmt_ctx, NULL, "avi", out_filename);
        goto exit;
    }

    if (!ofmt_ctx) 
    {
        goto exit;
    }

    ofmt = ofmt_ctx->oformat;

    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream)
    {
        printf("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        goto exit;
    }

    avctx = out_stream->codec;
    avctx->codec_type = AVMEDIA_TYPE_VIDEO;

    ret = av_dict_set(&dic, "qp", "0", 0);
    ret = av_dict_set(&dic, "bufsize", "1524000", 0);
    if (ret < 0)
    {
        printf("av_dict_set  bufsize fail \n");
        return -1;
    }

    ///*此处,需指定编码后的H264数据的分辨率、帧率及码率*/
    /* avctx->codec_id   = AV_CODEC_ID_H264;
     avctx->codec_type = AVMEDIA_TYPE_VIDEO;
     avctx->bit_rate = 2000000;
     avctx->width = 1280;
     avctx->height = 720;
     avctx->time_base.num = 1;
     avctx->time_base.den = 25;
     avctx->qmin = 10;
     avctx->qmax = 60;
     avctx->codec_tag = 0;
     avctx->has_b_frames = 0;*/

   /* 实际上并不起效，但是必须设置否则启动不起来  */
    out_stream->codecpar->codec_id = AV_CODEC_ID_H264;
    out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    out_stream->codecpar->width = 1920;
    out_stream->codecpar->height = 1080;
    out_stream->codecpar->codec_tag = 0;
    out_stream->codecpar->bit_rate = 8000000;
    out_stream->codecpar->format = AV_PIX_FMT_YUV420P;
    //out_stream->r_frame_rate = (AVRational){ 25, 1 };
    
    /* print output stream information*/
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output file '%s'\n", out_filename);
            goto exit;
        }
        printf("Open output file success!\n");
    }

    //写文件头（Write file header）
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("write avi file header failed\n");
        goto exit;
    }

    while (1)
    {
        av_init_packet(&pkt);
        ret = av_read_frame(ifmt_ctx, &in_pkt);
        if (ret < 0)
        {
            av_init_packet(&pkt);
            avformat_close_input(&ifmt_ctx);
            ret = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL);
            if (ret < 0)
            {
                return -1;
            }
            if (av_read_frame(ifmt_ctx, &in_pkt) != 0)
            {
                return -1;
            }
            
        }

        memcpy(frame_buf, in_pkt.data, in_pkt.size);
        printf("size = %d \n", in_pkt.size);

         if (in_pkt.size <= 4)
        {
            continue;
            av_packet_unref(&pkt);
        }

         int frame_type = H264_GetFrameType(frame_buf, in_pkt.size, 4);
         if (frame_type == H264_FRAME_I || frame_type == H264_FRAME_SI)
        { 
            // 判断该H264帧是否为I帧
            printf("####I frame ######\n");
            pkt.flags |= AV_PKT_FLAG_KEY;
            //getchar();
        }
        else
        {
            frame_type = H264_GetFrameType((unsigned char*)frame_buf, in_pkt.size, 3);
            if (frame_type == H264_FRAME_I || frame_type == H264_FRAME_SI)
            {
                // 判断该H264帧是否为I帧
                printf("11111####I frame ######\n");
                pkt.flags |= AV_PKT_FLAG_KEY;
            }
            else
            {
                /* p frame*/
                pkt.flags = 0;
            }

            }

        //pkt.dts = pkt.pts = AV_NOPTS_VALUE;
        pkt.dts = pkt.pts = frame_cnt;

        pkt.size = in_pkt.size; /*帧大小*/
        pkt.data = frame_buf; /*帧数据*/

        if (!pkt.data) 
        {
            printf("no data\n");
            continue;
        }

        //写入（Write）
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) 
        {
            av_strerror(ret, buf, 1024);
        }

        if (frame_cnt % 2)
            printf("Send frame[%d] \n", frame_cnt);
        frame_cnt++;

        av_packet_unref(&pkt);

        memset(frame_buf, 0, buffer_size);

        av_usleep(40000);
    }

    av_write_trailer(ofmt_ctx);

exit:

    if (frame_buf)
    {
        delete[] frame_buf;
        frame_buf = NULL;
    }
    /* close output */
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
     avformat_free_context(ofmt_ctx);

    return 0;
}


FfmpegRtmpStreamPusher::FfmpegRtmpStreamPusher(const char* in_filename, const char* out_filename)
{
    memset(m_in_filename, 0, MAX_FILE_URL_SIZE);
    memset(m_out_filename, 0, MAX_FILE_URL_SIZE);
    
#ifdef WIN32
    sprintf(m_in_filename, "%s", in_filename);
    sprintf(m_out_filename, "%s", out_filename);
#else
    snprintf(m_in_filename, MAX_FILE_URL_SIZE, "%s", in_filename);
    snprintf(m_out_filename, MAX_FILE_URL_SIZE, "%s", in_filename);
#endif

}

FfmpegRtmpStreamPusher::~FfmpegRtmpStreamPusher()
{
    DeInit();
}

bool FfmpegRtmpStreamPusher::Init()
{
    //av_log_set_level(AV_LOG_DEBUG);
    avcodec_register_all();
    av_register_all();

    //Network
    avformat_network_init();
    int ret = 0;

    AVDictionary *dic = NULL;
    ret = av_dict_set(&dic, "bufsize", "1024000", 0);
    if (ret < 0)
    {
        printf("av_dict_set  bufsize fail \n");
        return false;
    }

    ret = av_dict_set(&dic, "stimeout", "2000000", 0);
    if (ret < 0)
    {
        printf("av_dict_set  stimeout fail \n");
        return false;
    }

    ret = av_dict_set(&dic, "max_delay", "500000", 0);
    if (ret < 0)
    {
        printf("av_dict_set  max_delay fail \n");
        return false;
    }

    /* 设置为TCP方式传输 */
    /*ret = av_dict_set(&dic, "rtsp_transport", "tcp", 0);
    if (ret < 0)
    {
    printf("av_dict_set  rtsp_transport fail \n");
    return false;
    }*/

    if ((ret = avformat_open_input(&m_ifmt_ctx, m_in_filename, 0, 0)) < 0) 
    {
        printf("Could not open input file.");
        return false;
    }

    if ((ret = avformat_find_stream_info(m_ifmt_ctx, 0)) < 0) 
    {
        printf("Failed to retrieve input stream information");
        return false;
    }

    for (int i = 0; i < m_ifmt_ctx->nb_streams; i++)
    {
        if (m_ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoindex = i;
            break;
        }
    }

     av_dump_format(m_ifmt_ctx, 0, m_in_filename, 0);

      {
#if ENABLE_PUSH_RTSP_STREAM
        avformat_alloc_output_context2(&m_ofmt_ctx, NULL, "rtsp", m_out_filename); //RTSP
#else
        avformat_alloc_output_context2(&m_ofmt_ctx, NULL, "flv", m_out_filename); //RTMP
      //avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", out_filename);//UDP
#endif
        if (!m_ofmt_ctx)
        {
            printf("Could not create output context\n");
            ret = AVERROR_UNKNOWN;
            return false;
        }

        m_ofmt = m_ofmt_ctx->oformat;
        printf("ifmt_ctx->nb_streams = %d \n", m_ifmt_ctx->nb_streams);
        for (int i = 0; i < m_ifmt_ctx->nb_streams; i++) 
        {
            //Create output AVStream according to input AVStream
            AVStream *in_stream = m_ifmt_ctx->streams[i];
             AVStream *out_stream = avformat_new_stream(m_ofmt_ctx, in_stream->codec->codec);
            if (!out_stream)
            {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }

            //Copy the settings of AVCodecContext
            ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
            if (ret < 0)
            {
                printf("Failed to copy context from input to output stream codec context\n");
                return false;
            }

            out_stream->codec->codec_tag = 0;
        }

        //Dump Format------------------
        av_dump_format(m_ofmt_ctx, 0, m_out_filename, 1);
        
        //Open output URL
        if (!(m_ofmt->flags & AVFMT_NOFILE)) 
        {
            ret = avio_open(&m_ofmt_ctx->pb, m_out_filename, AVIO_FLAG_WRITE);
            if (ret < 0) 
            {
                printf("Could not open output URL '%s'", m_out_filename);
                return false;
            }
        }

        //Write file header
        ret = avformat_write_header(m_ofmt_ctx, NULL);
        if (ret < 0) 
        {
            printf("Error occurred when opening output URL\n");
            return false;
        }
        start_time = av_gettime();
    }

    return true;
}

bool FfmpegRtmpStreamPusher::ProcessLoop()
{
    while (1) 
    {
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        int ret = av_read_frame(m_ifmt_ctx, &m_pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if (m_pkt.pts == AV_NOPTS_VALUE)
        {
            //Write PTS
            AVRational time_base1 = m_ifmt_ctx->streams[m_videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(m_ifmt_ctx->streams[m_videoindex]->r_frame_rate);
            //Parameters
            m_pkt.pts = (double)(m_frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
            m_pkt.dts = m_pkt.pts;
            m_pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
         }

        //Important:Delay
        /* ffmpeg处理数据的速度很快，不做延时瞬间就能把所有数据发送出去，流媒体服务器接收不了 */
        if (m_pkt.stream_index == m_videoindex)
        {
            AVRational time_base = m_ifmt_ctx->streams[m_videoindex]->time_base;
            AVRational time_base_q = { 1, AV_TIME_BASE };
            int64_t pts_time = av_rescale_q(m_pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);
        }

        in_stream = m_ifmt_ctx->streams[m_pkt.stream_index];
        out_stream = m_ofmt_ctx->streams[m_pkt.stream_index];

        /* copy packet */
        //Convert PTS/DTS
        m_pkt.pts = av_rescale_q_rnd(m_pkt.pts, in_stream->time_base, out_stream->time_base, 
                                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        m_pkt.dts = av_rescale_q_rnd(m_pkt.dts, in_stream->time_base, out_stream->time_base, 
                              (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        m_pkt.duration = av_rescale_q(m_pkt.duration, in_stream->time_base, out_stream->time_base);
        m_pkt.pos = -1;

        //Print to Screen
        if (m_pkt.stream_index == m_videoindex)
        {
            printf("Send %8d video frames to output URL\n", m_frame_index);
            m_frame_index++;
        }

        //ret = av_write_frame(ofmt_ctx, &m_pkt);
        ret = av_interleaved_write_frame(m_ofmt_ctx, &m_pkt);
        if (ret < 0)
        {
            printf("Error muxing packet ret = %d\n", ret);
            char buf[128];
            memset(buf, 0, 128);
            av_strerror(ret, buf, 128);
            printf("error: %s\n", buf);
            av_free_packet(&m_pkt);
            continue;
        }
        
        av_free_packet(&m_pkt);
    }

    //Write file trailer
    av_write_trailer(m_ofmt_ctx);

    return true;
}

bool FfmpegRtmpStreamPusher::DeInit()
{
    if (m_ifmt_ctx)
    {
        avformat_close_input(&m_ifmt_ctx);
        m_ifmt_ctx = NULL;
    }

    /* close output */
    if (m_ofmt_ctx)
    {
        if (!(m_ofmt->flags & AVFMT_NOFILE))
            avio_close(m_ofmt_ctx->pb);

        avformat_free_context(m_ofmt_ctx);
        m_ofmt_ctx = NULL;
    }

    return true;
}

int read_buffer(void *opaque, uint8_t *buf, int buf_size);

/* 使能直接读取内存数据然后进行推流 */
#define  READ_FROME_MEM_BUFFER (1)

/* 推流测试 可推rtsp/rtmp流*/
int test_pusher_main( )
{
    AVOutputFormat *ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int videoindex = -1;
    int frame_index = 0;
    int64_t start_time = 0;

#ifndef READ_FROME_MEM_BUFFER
    in_filename  = "rtsp://localhost:556/test";//输入URL（Input file URL）
#else
    Read_Data read_handle("cuc_ieschool.flv");
#endif

    out_filename = "rtmp://localhost:1936/live/livestream";//输出 URL（Output URL）[RTMP]
    //out_filename = "rtp://233.233.233.233:6666";//输出 URL（Output URL）[UDP]

    //avfilter_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    avcodec_register_all();
    av_register_all();

    //Network
    avformat_network_init();

#ifdef READ_FROME_MEM_BUFFER
    ifmt_ctx = avformat_alloc_context();

    unsigned char* aviobuffer = (unsigned char *)av_malloc(32768);
    AVIOContext* avio = avio_alloc_context(aviobuffer, 32768, 0, &read_handle, Read_Data::read_buffer, NULL, NULL);

    /* Open an input stream and read the header. */
    ifmt_ctx->pb = avio;
    if (avformat_open_input(&ifmt_ctx, NULL, NULL, NULL) != 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

#else
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("Could not open input file.");
        goto end;
    }
#endif

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) 
    {
        printf("Failed to retrieve input stream information");
        goto end;
    }

    for (i = 0; i<ifmt_ctx->nb_streams; i++)
    if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        videoindex = i;
        break;
    }

    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }

#ifndef READ_FROME_MEM_BUFFER
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
#endif

    //Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename); //RTMP
    //avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", out_filename);//UDP
    if (!ofmt_ctx) 
    {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) 
    {
        //Create output AVStream according to input AVStream
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) 
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //Copy the settings of AVCodecContext
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) 
        {
            printf("Failed to copy context from input to output stream codec context\n");
            goto end;
        }

        out_stream->codec->codec_tag = 0;
        //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        //{
            //printf("###### set AV_CODEC_FLAG_GLOBAL_HEADER ######\n");
            //out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        //}
    }

    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    //Open output URL
    if (!(ofmt->flags & AVFMT_NOFILE)) 
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output URL '%s'", out_filename);
            goto end;
        }
    }

    //Write file header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) 
    {
        printf("Error occurred when opening output URL\n");
        goto end;
    }

    start_time = av_gettime();
    while (1) 
    {
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if (pkt.pts == AV_NOPTS_VALUE)
        {
            //Write PTS
            AVRational time_base1 = ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts = pkt.pts;
            pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
        }

        //Important:Delay
        if (pkt.stream_index == videoindex)
        {
            AVRational time_base = ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q = { 1, AV_TIME_BASE };
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);
        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, 
            (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                 (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

        pkt.pos = -1;
        //Print to Screen
        if (pkt.stream_index == videoindex)
        {
            printf("Send %8d video frames to output URL\n", frame_index);
            frame_index++;
        }

        //ret = av_write_frame(ofmt_ctx, &pkt);
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0)
        {
            printf("Error muxing packet ret = %d\n", ret);
            char buf[128];
            memset(buf, 0, 128);
            av_strerror(ret, buf, 128);
            printf("error: %s\n", buf);
            break;
        }

        av_free_packet(&pkt);
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);

    getchar();

end:

#ifdef READ_FROME_MEM_BUFFER
    av_freep(&avio->buffer);
    av_freep(&avio);
#endif

    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) 
    {
        printf("Error occurred.\n");
        return -1;
    }

    getchar();
    return 0;
}

Read_Data::Read_Data(const char* file_name)
{
    frd = fopen(file_name, "rb");
    read_thread = thread (read_main_loop, this);
}

Read_Data::~Read_Data()
{
    if (frd)
    {
        fclose(frd);
        frd = NULL;
    }
    thread_runing = 0;
    read_thread.join();
}

int Read_Data::read_main_loop(void* ptr)
{
    Read_Data* handle = (Read_Data*)ptr;
    int buf_size = 32708;
    char* buf = new char[buf_size];
    memset(buf, 0, buf_size);
     int true_size = 0;
     while (handle->thread_runing)
    {
        if (!feof(handle->frd))
        {
            true_size = fread(buf, 1, buf_size, handle->frd);
        }
        else
        {
            fseek(handle->frd, 0, SEEK_SET);
            true_size = fread(buf, 1, buf_size, handle->frd);
        }

        frame_info* info = new   frame_info;
        info->data = new char[true_size];
        memcpy(info->data, buf, true_size);
        info->data_size = true_size;
        handle->queue_mutex.lock();
        handle->frame_queue.push(info);
        handle->queue_mutex.unlock();

        memset(buf, 0, buf_size);
        av_usleep(45000);
    }

    delete[] buf;
    buf = NULL;
    return 0;
}

int Read_Data::read_buffer(void *opaque, uint8_t *buf, int buf_size)
{
    if (!opaque)
    {
        printf("fiel%s:%d -- Invalid parames \n", __FILE__, __LINE__);
        return -1;
    }

    Read_Data* handle = (Read_Data*)opaque;
    
     handle->queue_mutex.lock();
    if (!handle->frame_queue.empty())
    {
        frame_info* info = handle->frame_queue.front();
        memcpy(buf, info->data, info->data_size);
        handle->frame_queue.pop();
        handle->queue_mutex.unlock();
        int data_size = info->data_size;
        if (info)
        {
            delete[] info->data;
            delete info;
            info = NULL;
        }
        return data_size;
    }
    else
    {
        handle->queue_mutex.unlock();
        return 0;
    }
}

