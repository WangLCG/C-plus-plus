
#include "parse_file.h"
#include "xml/tinyxml2.h"

#include <iostream>
#include <vector>

using namespace tinyxml2;
using namespace std;

Parse_XML_File::Parse_XML_File(char* file_name)
{
    mXmlFileNmae = file_name;
}

//开始解析XML文件   
int Parse_XML_File::Do_XML_Parse()
{
    FFMPEG_XML_Context tmp_Context;
    if (mXmlFileNmae.length() == 0)
    {
        cout << "Invalid Xml File Name \n" << endl;
        getchar();
        return -1;
    }

    int ret = 0;
    XMLError errXml = this->docxml.LoadFile(mXmlFileNmae.c_str());
    if (XML_SUCCESS == errXml)
    {
        const XMLElement *element = docxml.FirstChildElement();
        for (const XMLElement * parent = element->FirstChildElement();
            parent;
            parent = parent->NextSiblingElement())
        {
            tmp_Context.IsEnableDoubleOutPut = 0;

            const XMLElement *params = parent->FirstChildElement();
            for (; params; params = params->NextSiblingElement())
            {

                if (strcmp(params->Name(), XML_NODE_CHANNEL_NUM) == 0)
                {
                    params->QueryStringAttribute("Num", &(tmp_Context.channel_num));
                    cout << tmp_Context.channel_num << endl;
                }
                else if (strcmp(params->Name(), XML_NODE_CROP_SIZE) == 0)
                {
                    params->QueryIntAttribute("Width", &tmp_Context.width);
                    params->QueryIntAttribute("Height", &tmp_Context.height);
                    params->QueryIntAttribute("X", &tmp_Context.coord_x);
                    params->QueryIntAttribute("Y", &tmp_Context.coord_y);
                    /*printf("width = %d height = %d x = %d , y = %d\n",
                        tmp_Context.width, tmp_Context.height, tmp_Context.coord_x, tmp_Context.coord_y);*/
                }
                else if (strcmp(params->Name(), XML_NODE_OUTPUT_SIZE) == 0)
                {
                    params->QueryIntAttribute("Width_Out", &tmp_Context.out_width);
                    params->QueryIntAttribute("Height_Out", &tmp_Context.out_height);
                    printf("out_width0 = %d out_height0 = %d \n", tmp_Context.out_width, tmp_Context.out_height);
                }
                else if (strcmp(params->Name(), XML_NODE_CROP_SIZE_1) == 0)
                {
                    tmp_Context.IsEnableDoubleOutPut = 1;  //使能输出两个视频

                    params->QueryIntAttribute("Width", &tmp_Context.width_1);
                    params->QueryIntAttribute("Height", &tmp_Context.height_1);
                    params->QueryIntAttribute("X", &tmp_Context.coord_x_1);
                    params->QueryIntAttribute("Y", &tmp_Context.coord_y_1);
                    /*printf("width = %d height = %d x = %d , y = %d\n",
                    tmp_Context.width, tmp_Context.height, tmp_Context.coord_x, tmp_Context.coord_y);*/
                }
                else if (strcmp(params->Name(), XML_NODE_OUTPUT_SIZE_1) == 0)
                {
                    tmp_Context.IsEnableDoubleOutPut = 1;  //使能输出两个视频

                    params->QueryIntAttribute("Width_Out", &tmp_Context.out_width_1);
                    params->QueryIntAttribute("Height_Out", &tmp_Context.out_height_1);
                    printf("out_width1 = %d out_height1 = %d \n", tmp_Context.out_width_1, tmp_Context.out_height_1);
                }
                else
                {
                    cout << "Parse parames Faile \n" << endl;
                    ret = -1;
                    break;
                }
            }

            if (ret == 0)
            {
                Params_List.push_back(tmp_Context);
            }

        }
    }

    return ret;
}

Parse_XML_File::~Parse_XML_File()
{
    vector<FFMPEG_XML_Context>().swap(Params_List);
}
