#include <stdio.h>
#include <string>
#include<string.h>
#include <iostream>
#include<stdlib.h>

#include "ffmpeg_stream_pusher.h"

using namespace std;

int main()
{
    test_pusher_main();
    //test_pusher_rtmp_main2();
    cout << "test_pusher_main success " << endl;
    //test_read_buffer_main();
    //cout << "test_read_buffer_main success " << endl;
    getchar();
    getchar();
    getchar();

#if (ENABLE_PUSH_RTSP_STREAM == 0)
    const char* in_filename  = "rtsp://172.16.41.249:554/test0.sdp";//输入URL（Input file URL）
    const char* out_filename = "rtmp://localhost:1935/myapp/livestream";//输出 URL（Output URL）[RTMP]
#else
    const char*  in_filename = "test.mp4";
    const char* out_filename = "rtsp://localhost:556/test";
#endif

    FfmpegRtmpStreamPusher* RtmpStreamPusher = new(std::nothrow) FfmpegRtmpStreamPusher(in_filename, out_filename);
    if (RtmpStreamPusher)
    {
        bool ret = RtmpStreamPusher->Init();
        if (ret)
        {
            RtmpStreamPusher->ProcessLoop();
        }
        else
        {
            RtmpStreamPusher->DeInit();
        }

        delete    RtmpStreamPusher;
        RtmpStreamPusher = NULL;
    }

    getchar();
    getchar();
    getchar();
    getchar();
    getchar();
    getchar();

    return 0;
}