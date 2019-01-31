#include "ffmpeg_test.h"
#include <stdio.h>
#include <iostream>
#include <string>

using namespace  std;

#define INBUF_SIZE    (300000)

AVCodecID strm_to_av_map_coding_type[STRM_CODEC_MAX] =
{
    AV_CODEC_ID_NONE,          //0
    AV_CODEC_ID_H264,          //1
    AV_CODEC_ID_H265,          //2
    AV_CODEC_ID_MPEG1VIDEO,    //3
    AV_CODEC_ID_MPEG2VIDEO,    //4
    AV_CODEC_ID_MJPEG,         //5
    AV_CODEC_ID_MPEG4          //6
};

AVPixelFormat strm_to_av_map_pixel_format[STRM_PIXFMT_MAX] =
{
    AV_PIX_FMT_NONE,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_RGB565,
    AV_PIX_FMT_ARGB,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_GRAY8,
    AV_PIX_FMT_NV21,
    AV_PIX_FMT_UYVY422,
    AV_PIX_FMT_YUYV422
};

static AVFrame*    mVideoFrame = NULL;
static AVFrame*      mFrameYUV = NULL;

// 颜色转换上下文
static struct SwsContext*  mImgConvertCtx = NULL;

///< 解码器
static AVCodec*            mVcodec = NULL;
///< 解码上下文
static AVCodecContext*     mAvContext = NULL;

static AVFormatContext*    mFormatCtx = NULL;  // 用于读取文件   

static int mVideoStreamIdx = 0;


typedef struct DecodeFrameParams 
{
    unsigned int mInputCodecType;
    unsigned int mOutputPixelFormat;
}DecodeFrameParams;

bool GetDecodedFrame(MediaFrame& rawFrame, DecodeFrameParams& params)
{
    
    int frameSize = av_image_get_buffer_size(strm_to_av_map_pixel_format[params.mOutputPixelFormat], mVideoFrame->width, mVideoFrame->height, 1);

    // 分配内存
    void* buf_ptr = NULL;
    buf_ptr = (void*)new(std::nothrow) char[frameSize];
    if (buf_ptr == NULL)
    {
        printf("decode memory allocation error! [buf_size = %d]\n", frameSize);
        return false;
    }

    if (mImgConvertCtx)
    {
        sws_freeContext(mImgConvertCtx);
    }

    av_image_fill_arrays(mFrameYUV->data, mFrameYUV->linesize, (uint8_t*)buf_ptr,
        strm_to_av_map_pixel_format[params.mOutputPixelFormat], mVideoFrame->width, mVideoFrame->height, 1);
    mImgConvertCtx = sws_getContext(mVideoFrame->width, mVideoFrame->height, mAvContext->pix_fmt,
        mVideoFrame->width, mVideoFrame->height, strm_to_av_map_pixel_format[params.mOutputPixelFormat], SWS_BICUBIC, NULL, NULL, NULL);

    sws_scale(mImgConvertCtx, (const unsigned char* const*)mVideoFrame->data,
        mVideoFrame->linesize, 0, mVideoFrame->height,
        mFrameYUV->data, mFrameYUV->linesize);

    //mWidth  = mVideoFrame->width;
    //mHeight = mVideoFrame->height;

    // 配置输出信息
    rawFrame.frameBuf = (unsigned char*)buf_ptr;
    rawFrame.frameSize = frameSize;

    return true;
}

int test_decode(unsigned int mInputCodecType, unsigned int mOutputPixelFormat, 
    string &filename, string &error)
{
    FILE* fp_YUV = fopen("decode_out.yuv", "wb+");
    if (!fp_YUV)
    {
        perror("open out.yuv :");
        return -1;
    }

    av_register_all();

    mVideoFrame = av_frame_alloc();
    mFrameYUV   = av_frame_alloc();

    bool                mHasKeyFrame = false;

    AVPacket            mPktPacket;

    long long    mDecodedBytes = 0;

    int ret = avformat_open_input(&mFormatCtx, filename.c_str(), NULL, NULL);
    if (ret < 0)
    {
        printf("avformat_open_input fail \n");
        error = "avformat_open_input fail";

        av_log(NULL, AV_LOG_ERROR, "Cannot open file: %s.\n", filename.c_str());
        return false;
    }
    if (avformat_find_stream_info(mFormatCtx, NULL) < 0)
    {
        return false;
    }

    av_dump_format(mFormatCtx, 0, filename.c_str(), 0);

    //获取视频的编码信息   
    for (uint32_t i = 0; i < mFormatCtx->nb_streams; ++i)
    {
        if (mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            mVideoStreamIdx = i;
            break;
        }
    }

    // 寻找解码器
    mVcodec = avcodec_find_decoder(strm_to_av_map_coding_type[mInputCodecType]);
    mAvContext = avcodec_alloc_context3(mVcodec);
    if (!mVcodec || !mAvContext)
    {
        //DecoderDeinit();
        return false;
    }

    ///不初始化解码器context会导致MP4封装的mpeg4码流解码失败    
    ret = avcodec_parameters_to_context(mAvContext, mFormatCtx->streams[mVideoStreamIdx]->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing the decoder context.\n");
        //exit_program(1);
    }

    // 打开解码器
    mAvContext->pix_fmt = strm_to_av_map_pixel_format[mOutputPixelFormat];
    if (avcodec_open2(mAvContext, mVcodec, NULL) != 0)
    {
        //DecoderDeinit();
        return false;
    }

    //循环读入H264数据  
    AVPacket h264Pack;
    
    while (1)
    {
        av_init_packet(&h264Pack);
        int ret = av_read_frame(mFormatCtx, &h264Pack);
        if (ret != 0)
        {
            error = "Read a frame failed";
            av_packet_unref(&h264Pack);
            return false;
        }
        else if (h264Pack.stream_index != mVideoStreamIdx)
        {
            // not a video packet, skip it in this version
            av_packet_unref(&h264Pack);
            continue;
        }

        mHasKeyFrame = true;
        if (mHasKeyFrame)
        {

            // 初始化待解码包
            av_init_packet(&mPktPacket);
            mPktPacket.data = (uint8_t*)h264Pack.data;     // 用于解码的压缩视频帧数据，mBitStreamBuf中包含pps数据
            mPktPacket.size = h264Pack.size;
            mPktPacket.pts = h264Pack.pts;
            mPktPacket.dts = h264Pack.dts;

            mDecodedBytes += mPktPacket.size;

            // 发送待解码包
            if (avcodec_send_packet(mAvContext, &mPktPacket))
            {
                error = "send packet failed";
                mHasKeyFrame = false;
                av_packet_unref(&mPktPacket);
                return false;
            }

            av_packet_unref(&mPktPacket);

            // 接收解码数据
            int ret = avcodec_receive_frame(mAvContext, mVideoFrame);
            if (ret != 0)
            {
                if (ret == AVERROR(EAGAIN))
                {
                    // 暂时没有输出，需要更多输入
                    error = "need more data";
                    //return false;
                    continue;
                }
                //error = "receive frame failed";
                // 输出数据接收失败
                //mHasKeyFrame = false;
                //return false;
            }

            DecodeFrameParams params;
            params.mInputCodecType = mInputCodecType;
            params.mOutputPixelFormat = mOutputPixelFormat;

            MediaFrame rawFrame;
            // 获取解码后的YUV数据
            GetDecodedFrame(rawFrame, params);
            //int mFrameRate = mAvContext->framerate.num;

            // 写文件保存视频数据
            fwrite(rawFrame.frameBuf, rawFrame.frameSize, 1, fp_YUV);
            fflush(fp_YUV);

            if (rawFrame.frameBuf)
            {
                delete[] rawFrame.frameBuf;
                rawFrame.frameBuf = NULL;
            }
        }
    }

    fclose(fp_YUV);
    avformat_close_input(&mFormatCtx);
    //printf("[strmsdk] decoder[%s] open successful.\n", mVcodec->name);
}


int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) 
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
    {
        return 0;
    }

    while (1) 
    {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
            NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

int test_encode(int argc, char* argv[])
{
    AVFormatContext* pFormatCtx;
    AVOutputFormat* fmt;
    AVStream* video_st;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;
    AVPacket pkt;
    uint8_t* picture_buf;
    AVFrame* pFrame;
    int picture_size;
    int y_size;
    int framecnt = 0;
    //FILE *in_file = fopen("src01_480x272.yuv", "rb"); //Input raw YUV data 
    FILE *in_file = fopen("ds_352x288.yuv", "rb");   //Input raw YUV data
    int in_w = 352, in_h = 288;                      //Input data's width and height
    int framenum = 50;                                   //Frames to encode
    //const char* out_file = "src01.h264";              //Output Filepath 
    //const char* out_file = "src01.ts";
    //const char* out_file = "src01.hevc";
    const char* out_file = "ds.h264";

    av_register_all();
    //Method1.
    pFormatCtx = avformat_alloc_context();
    //Guess Format
    fmt = av_guess_format(NULL, out_file, NULL);
    pFormatCtx->oformat = fmt;

    //Method 2.
    //avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
    //fmt = pFormatCtx->oformat;

    //Open output URL
    if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Failed to open output file! \n");
        return -1;
    }

    video_st = avformat_new_stream(pFormatCtx, 0);
    //video_st->time_base.num = 1; 
    //video_st->time_base.den = 25;  

    if (video_st == NULL) {
        return -1;
    }
    //Param that must set
    pCodecCtx = video_st->codec;
    //pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 25;   //I帧间隔   

    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;  //time_base一般是帧率的倒数，但不总是  
    pCodecCtx->framerate.num = 25;  //帧率  
    pCodecCtx->framerate.den = 1;

    ///AVFormatContext* mFormatCtx 
    ///mBitRate   = mFormatCtx->bit_rate;   ///码率存储位置  
    ///mFrameRate = mFormatCtx->streams[stream_id]->avg_frame_rate.num;
    
    
    //H264
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;

    //Optional Param
    pCodecCtx->max_b_frames = 0;  //不要B帧   

    // Set Option
    AVDictionary *param = 0;
    //H.264
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
        //av_dict_set(&param, "profile", "main", 0);
    }
    //H.265
    if (pCodecCtx->codec_id == AV_CODEC_ID_H265) {
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zero-latency", 0);
    }

    //Show some Information
    av_dump_format(pFormatCtx, 0, out_file, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        printf("Can not find encoder! \n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
        printf("Failed to open encoder! \n");
        return -1;
    }


    pFrame = av_frame_alloc();
    picture_size = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
    picture_buf = (uint8_t *)av_malloc(picture_size);
    //avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    av_image_fill_arrays(pFrame->data, pFrame->linesize, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);

    //Write File Header
    avformat_write_header(pFormatCtx, NULL);

    av_new_packet(&pkt, picture_size);

    y_size = pCodecCtx->width * pCodecCtx->height;

    for (int i = 0; i < framenum; i++) 
    {
        int ret = 0;
        //Read raw YUV data
        if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0) 
        {
            printf("Failed to read raw data! \n");
            return -1;
        }
        else if (feof(in_file)) {
            break;
        }

        int got_picture = 0;

        pFrame->data[0] = picture_buf;              // Y
        pFrame->data[1] = picture_buf + y_size;      // U 
        pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
                                                         //PTS
                                                         //pFrame->pts=i;
        pFrame->pts = i;
        pFrame->width = pCodecCtx->width;
        pFrame->height = pCodecCtx->height;
        pFrame->format = AV_PIX_FMT_YUV420P;

        //Encode   旧版接口   
        /*ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
        if (ret < 0) {
            printf("Failed to encode! \n");
            return -1;
        }
        if (got_picture == 1) {
            printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
            framecnt++;
            pkt.stream_index = video_st->index;
            ret = av_write_frame(pFormatCtx, &pkt);
            av_free_packet(&pkt);
        }*/

        ret = avcodec_send_frame(pCodecCtx, pFrame);
        if (ret < 0)
        {
            fprintf(stderr, "Error sending a frame for encoding\n");
            return -1;
        }

        while (ret == 0)
        {
            //framecnt++;
            ret = avcodec_receive_packet(pCodecCtx, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                framecnt++;
                break;
            }
            //framecnt++;
            pkt.stream_index = video_st->index;
            ret = av_write_frame(pFormatCtx, &pkt);
            av_packet_unref(&pkt);
        }

    }

    //Flush Encoder
    int ret = flush_encoder(pFormatCtx, 0);
    if (ret < 0) {
        printf("Flushing encoder failed\n");
        return -1;
    }

    //Write file trailer
    av_write_trailer(pFormatCtx);

    //Clean
    if (video_st) {
        avcodec_close(video_st->codec);
        av_free(pFrame);
        av_free(picture_buf);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    fclose(in_file);

    return 0;
}

