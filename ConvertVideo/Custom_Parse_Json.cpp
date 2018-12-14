#include "Custom_Parse_Json.h"
#include "jsoncpp/json/json.h"

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

Parse_FMPEG_Json_File::Parse_FMPEG_Json_File(char* filename)
{
    Json_File_Name = filename;
}

Parse_FMPEG_Json_File::Parse_FMPEG_Json_File()
{

}

int Parse_FMPEG_Json_File::Do_Json_Parse(string& Parse_Context)
{
    if (!Json_File_Name.length())
    {
        cout << " Json file name is null " << endl;
        return -1;
    }

    ifstream ifs(Json_File_Name.c_str());
    if (!ifs.is_open())
    {
        cout << "Error open json file : " << Json_File_Name << endl;
        return -1;
    }

    if (json_reader.parse(ifs, root))
    {
        string code;
        code = root["files"].isNull() ? "NULL" : root["uploadid"].asString();

        //访问节点，如果节点存在返回节点值，否则返回默认值
        code = root.get("uploadid", "null").asString();

        cout << "code : " << code << endl;
        
        //输出  
        writer["uploadid"] = code;
        writer["code"] = root["code"].isNull() ? 1 : root["code"].asInt();
        Parse_Context = writer.toStyledString();

        //得到files 数组个数
        int filesize = root["files"].size();
        cout << "filesize : " << filesize << endl;

        for (int i = 0; i < filesize; ++i)
        {
            Json::Value val_image = root["files"][i]["images"];
            int image_size = val_image.size();
            for (int j = 0; j < image_size; ++j)
            {
                string type = val_image[j]["type"].asString();
                string url = val_image[j]["url"].asString();
                
                cout << j <<"-type : " << type << endl;
                cout << j <<"-url : "  << url << endl;
            }
        }
    }
    else
    {
        cout << "Error parse json file : " << Json_File_Name << endl;
    }


    ifs.close();
    return 0;

}

int Parse_FMPEG_Json_File::Do_String_Json_Parse(string& json)
{
    if (!json.length())
    {
        cout << "Error json string is null " << endl;
        return -1;
    }

    if (json_reader.parse(json, root))
    {
        string upload_id = root["uploadid"].asString();
        int code = root["code"].asInt();

        cout << "code: " << code << endl;
        cout << "upload_id: " << upload_id << endl;
    }
    else
    {
        cout << "parse json string faile " << endl;
        return -1;
    }

    return 0;
}
