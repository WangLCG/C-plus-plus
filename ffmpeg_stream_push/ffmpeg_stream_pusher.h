////////////////////////////////////////////////////////////////////////
/// @file       ffmpeg_stream_pusher.h
/// @brief      ffempg 推流测试demo 
/// @details    包含读文件推rtsp/rtmp流,读rtsp流推rtsp流/rtmp流, 读rtmp流推rtsp/rtmp流， 读内存数据推rtsp/rtmp流
/// @author     
/// @version    0.1
/// @date       2019/08/20
/// @copyright  (c) 1993-2019
////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#define __STDC_CONSTANT_MACROS
#ifdef _WIN32
//Windows
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif
#endif

#include <queue>
#include <thread>
#include <mutex>

using namespace std;

#define ENABLE_PUSH_RTSP_STREAM  (1)

int test_pusher_main();
/* 文件URL最大长度 */
#define MAX_FILE_URL_SIZE (128)

enum H264_FRAME_TYPE
{
    H264_FRAME_NALU = 0,        ///< 参数集类NALU
    H264_FRAME_I = 1,        ///< I帧
    H264_FRAME_SI = 2,        ///< I帧
    H264_FRAME_P = 3,        ///< P帧
    H264_FRAME_SP = 4,        ///< P帧
    H264_FRAME_B = 5,        ///< B帧
    H264_FRAME_UNKNOWN
};

//nal类型
enum H264_NALU_TYPE
{
    H264_NAL_UNKNOWN = 0,
    H264_NAL_SLICE = 1,
    H264_NAL_SLICE_DPA = 2,
    H264_NAL_SLICE_DPB = 3,
    H264_NAL_SLICE_DPC = 4,
    H264_NAL_SLICE_IDR = 5,    /* ref_idc != 0 */
    H264_NAL_SEI = 6,    /* ref_idc == 0 */
    H264_NAL_SPS = 7,
    H264_NAL_PPS = 8
    /* ref_idc == 0 for 6,9,10,11,12 */
};

typedef struct
{
    unsigned char*  p_start;
    unsigned char*  p;
    unsigned char*  p_end;
    int             i_left;
}
bs_t;

typedef struct frame_info
{
    char* data   ;
    int data_size ;
    frame_info()
    {
        data = NULL;
        data_size = 0;
    }
}frame_info;

int test_pusher_rtmp_main2();

class Read_Data
{
public:
    Read_Data(const char* file_name);
    ~Read_Data();
    static int read_buffer(void *opaque, uint8_t *buf, int buf_size);
    static int read_main_loop(void* handle);

private:
     mutex queue_mutex;
    queue<frame_info*> frame_queue;
    thread read_thread;
    bool thread_runing = 1;

    char file_name[128];
    FILE* frd = NULL;
};

class FfmpegRtmpStreamPusher
{
public:
    FfmpegRtmpStreamPusher(const char* in_filename, const char* out_filename);
    ~FfmpegRtmpStreamPusher();
    /* 初始化 成功返回--true*/
    bool Init();
    /* 反初始化 成功返回--true*/
    bool DeInit();

    bool ProcessLoop();
private:

    AVOutputFormat *m_ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *m_ifmt_ctx = NULL, *m_ofmt_ctx = NULL;

    AVPacket m_pkt;
    char m_in_filename[MAX_FILE_URL_SIZE];     //输入文件url
    char m_out_filename[MAX_FILE_URL_SIZE];    //输出url

    int m_videoindex   = -1;
    int m_frame_index  = 0;
    int64_t start_time = 0;
};

int test_read_buffer_main();
int read_buffer(void *opaque, uint8_t *buf, int buf_size);
