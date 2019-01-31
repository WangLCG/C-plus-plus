#ifndef _FFMPEG_TEST_
#define _FFMPEG_TEST_
extern "C" {
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/file.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#include <string>
using namespace std;

enum STRM_CODEC_TYPE
{
    STRM_CODEC_NONE = 0x00,
    STRM_CODEC_MJPG = 0x01,
    STRM_CODEC_MPEG4 = 0x02,
    STRM_CODEC_H263 = 0x03,
    STRM_CODEC_H264 = 0x04,
    STRM_CODEC_VP8 = 0x05,
    STRM_CODEC_H265 = 0x06,
    STRM_CODEC_MAX = 0xFF
};

enum STRM_PIXEL_FORMAT
{
    STRM_PIXFMT_UNKNOWN = 0x00,
    STRM_PIXFMT_RGB24 = 0x01,
    STRM_PIXFMT_RGB32 = 0x02,
    STRM_PIXFMT_RGB565 = 0x03,
    STRM_PIXFMT_ARGB32 = 0x04,
    STRM_PIXFMT_YUV420P = 0x05,
    STRM_PIXFMT_YUV422P = 0x06,
    STRM_PIXFMT_YUV444P = 0x07,
    STRM_PIXFMT_GRAY = 0x08,
    STRM_PIXFMT_YV12 = 0x09,
    STRM_PIXFMT_UYVY = 0x0A,
    STRM_PIXFMT_YUYV = 0x0B,
    STRM_PIXFMT_MAX = 0xFF
};

enum STRM_FRAME_TYPE
{
    STRM_FRAME_TYPE_UNKNOWN = 0, ///< Undefined
    STRM_FRAME_TYPE_I,     ///< Intra
    STRM_FRAME_TYPE_P,     ///< Predicted
    STRM_FRAME_TYPE_B,     ///< Bi-dir predicted
    STRM_FRAME_TYPE_S,     ///< S(GMC)-VOP MPEG4
    STRM_FRAME_TYPE_SI,    ///< Switching Intra
    STRM_FRAME_TYPE_SP,    ///< Switching Predicted
    STRM_FRAME_TYPE_BI,    ///< BI type
};

struct MediaFrame
{
    
    /// 帧数据长度
    int             frameSize;
    /// 帧数据缓冲区
    unsigned char*   frameBuf;
    MediaFrame()
        :  frameSize(0), frameBuf(NULL)
    {}

    ~MediaFrame()
    {

    }

};

int test_decode(unsigned int mInputCodecType, unsigned int mOutputPixelFormat,
    string &filename, string &error);

int test_encode(int argc, char* argv[]);

#endif
