#include <stdio.h>
#include <string>
#include<windows.h>
#include<string.h>
#include<io.h>
#include <iostream>
#include<stdlib.h>
#include<vector>
#include <fstream>
#include <cassert>

#include "ffmpeg_test.h"

using namespace std;

int main()
{
    
    //decodeVideo_test(0, NULL);
    //mp4_h264_decode_main(0 ,NULL);

    //解码demo avi(MPEG4)-> YUV
    string error;
    string filename;
    //cout << "Input video: " << endl;
    //cin >> filename;
    //test_decode(6, 5, filename, error);
    //cout << "error: " << error << endl;

    //编码  (yuv -> h264)
    test_encode(0, NULL);

    //test_Decode_Encode_Dec(error);
    ///cout << "error: " << error << endl;

    //test_Decode_Encode_Enc(error);
    //test_Decode_Encode_DecoThenEnco(error);
    cout << "error: " << error << endl;

    getchar();
    getchar();
    getchar();
    getchar();
    getchar();
    getchar();
    return 0;

}