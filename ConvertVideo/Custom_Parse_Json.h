#ifndef _CUSTOM_PARSE_JSON_
#define _CUSTOM_PARSE_JSON_

#include "jsoncpp/json/json.h"
#include <string>

using namespace std;
using namespace Json;

class Parse_FMPEG_Json_File
{
public:
    string Json_File_Name;
    
    Json::Reader json_reader;  //解析json用  
    Json::Value  root;         //Json::Value是重要的一种类型，可以代表任意类型，如int,string, char,...
    Json::Value  writer;

    Parse_FMPEG_Json_File(char* filename);
    Parse_FMPEG_Json_File();

    //由文件解析JSON  ,成功则返回以字符串形式保存的文本信息
    int Do_Json_Parse(string& Parse_Context);

    //解析JSON格式字符串  
    int Do_String_Json_Parse(string& json);
};
#endif
