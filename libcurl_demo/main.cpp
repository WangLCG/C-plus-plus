#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include "ftpclient.h"
#include "tinyxml2.h"
#include <ctime>
#include <sstream>
#include <vector>

using namespace std;
using namespace tinyxml2;

/* 按照 delim分割字符串src */
void split(string src, char delim, vector<string>& array)
{
    stringstream tmp_strream(src);
    string tmp;
    while(getline(tmp_strream, tmp, delim))
    {
        array.push_back(tmp);
    }
}

bool CreateXML(string& xml_str)
{
    XMLDocument doc;

    /* 添加声明 */
    XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);

    /* 创建根节点 */
    XMLElement* root = doc.NewElement("root");
    doc.InsertEndChild(root);

    /* 创建教师子节点 */
    XMLElement* childNodeTeacher = doc.NewElement("Teacher");
    XMLText* contentTeacher = doc.NewText("张三");
    childNodeTeacher->InsertFirstChild(contentTeacher);
    root->InsertEndChild(childNodeTeacher);

    /* 创建课程名子节点 */
    XMLElement* childNodeCourse = doc.NewElement("Course_name");
    XMLText* contentCourse = doc.NewText("数学");
    childNodeCourse->InsertFirstChild(contentCourse);
    root->InsertEndChild(childNodeCourse);

    /* 创建课程名子节点 */
    XMLElement* childNodeStartTime = doc.NewElement("Start_time");
    XMLText* contentStartTime = doc.NewText("2020-03-15 19:15:41");
    childNodeStartTime->InsertFirstChild(contentStartTime);
    root->InsertEndChild(childNodeStartTime);

    /* 创建视频文件大小子节点 */
    XMLElement* childNodeSize = doc.NewElement("Movie_size");
    XMLText* contentSize = doc.NewText("15585858");
    childNodeSize->InsertFirstChild(contentSize);
    root->InsertEndChild(childNodeSize);

    /* 创建视频时长子节点 */
    XMLElement* childNodeTimeLength = doc.NewElement("Time_lenth");
    XMLText* contentTimeLength = doc.NewText("15585858");
    childNodeTimeLength->InsertFirstChild(contentTimeLength);
    root->InsertEndChild(childNodeTimeLength);

    /* 创建live_id子节点 */
    XMLElement* childNodeLiveID = doc.NewElement("Live_id");
    XMLText* contentLiveID = doc.NewText("5");
    childNodeLiveID->InsertFirstChild(contentLiveID);
    root->InsertEndChild(childNodeLiveID);

    XMLPrinter printer;
    doc.Print(&printer);
    xml_str = printer.CStr();
    cout << "xml_str: " << xml_str << endl;
}

int main()
{
    string xml;
    CreateXML(xml);
    
    int nRet = -1;
    char *pSendData = xml.c_str();
    long lRespCode = 0;
    stFtpClientInfo stFtpClienter;
    memset(&stFtpClienter, 0, sizeof(stFtpClienter));
  
    char* local_file_path = "/mnt/nfs/20200401_开始.mp4";

    vector<string> tmp_array;
    string src_path = local_file_path;
    split(src_path, '/', tmp_array);
    size_t array_size = tmp_array.size();
    string str_file_name = tmp_array[array_size - 1];

    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    char server_path[48] = {'\0'};
    char* split = strstr(str_file_name.c_str(), "_");

    /* 服务器端文件夹名字 */
    char server_dir_name[20] = {'\0'};
    memcpy(server_dir_name, str_file_name.c_str(), split - str_file_name.c_str());
    
    /* 上传到云服务器的路径 */
    snprintf(server_path, 48, "%d%02d%02d/%s/%s", 1900+ltm->tm_year,
        1 + ltm->tm_mon, ltm->tm_mday, server_dir_name, str_file_name.c_str());

    cout << "server_path: " << server_path << endl;

    char xml_name[48]={'\0'};
    const char* splitxml = strstr(str_file_name.c_str(), ".");
    memcpy(xml_name, str_file_name.c_str(), splitxml - str_file_name.c_str());
    cout << "xml_pre_name: " << xml_name << endl;

    memcpy(stFtpClienter.chServerPath, server_path, strlen(server_path));

    //char* server_ip = "172.16.41.230";
    char* server_ip = "192.168.1.103";
    char* user_name = "root";
    char* passwd = "itc123456";

    memcpy(stFtpClienter.chServerIP, server_ip, strlen(server_ip));
    memcpy(stFtpClienter.chUserName, user_name, strlen(user_name));
    memcpy(stFtpClienter.chUserPassWord, passwd, strlen(passwd));

    stFtpClienter.nServerPort               = 21;  
    stFtpClienter.use_proxy                 = 0;                          //是否选择http代理服务器，0-不选择
    stFtpClienter.stFtpUser.nOperation      = FTP_OPERATION_UPLOAD_FILE;  //选择是否上送内存数据或文件 FTP_OPERATION_UPLOAD_MEM
    //stFtpClienter.stFtpUser.nOperation      = FTP_OPERATION_UPLOAD_MEM;
    stFtpClienter.stFtpUser.pRcvBuffer      = NULL;
    stFtpClienter.stFtpUser.nRcvBufLen      = 0;
    stFtpClienter.stFtpUser.nRcvSize        = 0;
    stFtpClienter.stFtpUser.nHaveSendLen    = 0;
    stFtpClienter.stFtpUser.pSendBuffer     = pSendData; //发送数据
    stFtpClienter.stFtpUser.nSedSize        = strlen(pSendData); //发送数据长度
    
    snprintf(stFtpClienter.stFtpUser.cSndFilePathName, 128, "%s", local_file_path);

    /*上送至ftp服务器*/  
    nRet = ftpClient_ftpUpload(&stFtpClienter,&lRespCode);
    if(nRet<0)
    {
        printf("ftpClient_ftpUpload failed\n");
        return -1;
    }

    return 0;  
}
