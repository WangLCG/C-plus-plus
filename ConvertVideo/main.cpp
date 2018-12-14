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

#include "parse_file.h"
#include "Custom_Parse_Json.h"

using namespace std;
#define EXE_FILE  "ffmpeg.exe -threads 2 -i %s -aspect 11:9 -vf crop=%d:%d:%d:%d -s %d*%d-r 25 -g 25 -bf 0 -b 3.2M -y %s"

typedef struct Crop_Context
{
    unsigned int channel_num;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
}Crop_Context;

void exe_ffmpeg(string& input_file, string& output_file, vector<FFMPEG_XML_Context>& ffmpeg_list)
{

    //判断输入文件是否存在 
    fstream _file;
    _file.open(input_file.c_str(), ios::in);
    if (!_file)
    {
        cout << "File " << input_file << "don't exist or has been remove" << endl;
        return;
    }

    char name[512];
    memset(name, 0, 512 * sizeof(char));

    for (auto it : ffmpeg_list)
    {
        //找出文件对应的参数  
        if (input_file.find(it.channel_num) != string::npos )  //input_file.substr(0, 4) == it.channel_num)
        {
            sprintf(name, EXE_FILE, input_file.c_str(), it.width, it.height,
                it.coord_x, it.coord_y, it.out_width, it.out_height,
                output_file.c_str());

            WinExec(name, SW_SHOW);

            if (it.IsEnableDoubleOutPut == 1)
            {

                output_file = input_file.substr(0, input_file.length() - 4);
                output_file += "_1.avi";

                memset(name, 0, 512 * sizeof(char));
                sprintf(name, EXE_FILE, input_file.c_str(), it.width_1, it.height_1,
                    it.coord_x_1, it.coord_y_1, it.out_width_1, it.out_height_1,
                    output_file.c_str());

                WinExec(name, SW_SHOW);
            }
        }
    }
}

int get_output_file_name(string& input_file, string& out_file_name)
{
    if (input_file.length() == 0)
    {
        return -1;
    }

    out_file_name = input_file.substr(0, input_file.length() - 4);
    out_file_name += ".avi";

   /* fstream f1;
    f1.open(out_file_name);
    if (!f1)
    {
        return 0;
    }
    else
    {
        out_file_name = input_file.substr(0, input_file.length() - 4);
        out_file_name += "_1.avi";
        f1.close();
    }*/

    return 0;
}

int  list_dir_mp4file(string& path , vector<string>& files) 
{
    long hFile = 0;
    struct _finddata_t fileInfo;

    string path_name;

    /// \\*表示遍历所有类型  
    if ( (hFile = _findfirst( path_name.assign(path).append("\\*").c_str(), &fileInfo)) == -1)
    {
        cout<<"error : path_name = " << path_name<<endl;
        perror("_findfirst");
        return -1;
    }

    do 
    {
        //判断是文件夹还是文件  
        if ((fileInfo.attrib & _A_SUBDIR))  //文件夹
        {   
            if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0)
            {
                //files.push_back(path_name.assign(path).append("\\").append(fileInfo.name));
                list_dir_mp4file(path_name.assign(path).append("\\").append(fileInfo.name), files);
            }
        }
        else
        {
            if (strstr(fileInfo.name, ".mp4"))
            {
                //cout << fileInfo.name << (fileInfo.attrib & _A_SUBDIR ? " [folder]" : " [file]") << endl;
                files.push_back(path_name.assign(path).append("\\").append(fileInfo.name));
            }
        }

    } while (_findnext(hFile, &fileInfo) == 0);

    _findclose(hFile);

    return 0;
}

int main()
{
    
    string Parse_Context;
    Parse_FMPEG_Json_File parse_json_file("test.json");
    parse_json_file.Do_Json_Parse(Parse_Context);
    
    cout << Parse_Context << endl;
    //string str = "{\"uploadid\": \"UP00000\", \"code\":100, \"msg\":\"test\"}";
    parse_json_file.Do_String_Json_Parse(Parse_Context);

    getchar();
    return 0;

    string path; 
    
    cout << "input dir: "<< endl;
    cin >> path;

    FILE* wfd = fopen("convert.log","wb+");
    if (!wfd)
    {
        cout << "open file convert.log error" << endl;
        perror("Fail: ");
        getchar();
        getchar();
        return -1;
    }

    //vector<Crop_Context> crop_contex;
    //Init_Crop_Value(crop_contex);
    //for (auto it : crop_contex)
    //{
    //    cout << "chan_num : " << it.channel_num << " width " << it.width << endl;
    //}

    Parse_XML_File ffmpeg_parse("test.xml");
    int ret = ffmpeg_parse.Do_XML_Parse();

    if (ret == -1)
    {
        printf("parse xml faile \n");
        if (!wfd)
        {
            fclose(wfd);
            wfd = NULL;
        }
        getchar();
        return -1;
    }

    vector<string> files;
    ret = list_dir_mp4file(path, files);
    if (ret == -1)
    {
        printf("list_dir_mp4file faile \n");
        if (!wfd)
        {
            fclose(wfd);
            wfd = NULL;
        }
        getchar();
        return -1;
    }

    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
    {
        std::string out_file;
        get_output_file_name(*it, out_file);
        fprintf(wfd, "Convert file %s to %s\n", it->c_str(), out_file.c_str());

        //MP4转AVI
        //exe_ffmpeg(*it, out_file, crop_contex);
       // exe_ffmpeg(*it, out_file, ffmpeg_parse.Params_List);
    }

    if (wfd)
    {
        fclose(wfd);
        wfd = NULL;
    }

    //vector<Crop_Context>().swap(crop_contex);
    getchar();
    getchar();
    getchar();
    return 0;
}