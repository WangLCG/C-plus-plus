#ifndef _PARSE_FILE_H
#define _PARSE_FILE_H

#include "xml/tinyxml2.h"
#include<string>
#include <vector>

using namespace tinyxml2;
using namespace std;

#define XML_NODE_CHANNEL_NUM  "Channel"
#define XML_NODE_CROP_SIZE    "Crop_Size"
#define XML_NODE_OUTPUT_SIZE  "OutPut_Size"
#define XML_NODE_CROP_SIZE_1    "Crop_Size_1"
#define XML_NODE_OUTPUT_SIZE_1  "OutPut_Size_1"

typedef struct FFMPEG_XML_Context
{
    const char* channel_num;
    
    //一个输入视频最多允许输出两个视频  
    int coord_x;  //裁剪起始坐标点x  
    int coord_y;  //裁剪起始坐标点y  
    int width;
    int height;     //裁剪输出尺寸1  
    int out_width;  //最终输出尺寸1  
    int out_height;

    int coord_x_1;
    int coord_y_1;
    int width_1;
    int height_1;      //裁剪输出尺寸2  
    int out_width_1;   //最终输出尺寸2  
    int out_height_1;

    int IsEnableDoubleOutPut;  //1-使能输出两个视频， 0-禁止输出两个视频   
}FFMPEG_XML_Context;

class Parse_XML_File 
{

public:
    vector<FFMPEG_XML_Context> Params_List;   //xml文本信息存放   

    tinyxml2::XMLDocument docxml;
    Parse_XML_File(char* file_name);
    int Do_XML_Parse();
    ~Parse_XML_File();
    string  mXmlFileNmae;
};
#endif


