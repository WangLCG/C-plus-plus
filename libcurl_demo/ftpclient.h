
#ifndef __FTPCLIENT_H  
#define __FTPCLIENT_H      
  
#ifdef __cplusplus   
extern "C" {   
#endif  
/***************************************************************** 
* 宏定义 
******************************************************************/  
#define FTP_OPERATION_UPLOAD_MEM            0   //上传内存数据  
#define FTP_OPERATION_UPLOAD_FILE           1   //上传文件数据  
  
/***************************************************************** 
* 结构、枚举定义 
******************************************************************/  
typedef struct _stFtpUserInfo  
{  
    int nOperation;             //操作选择  
    int nHaveSendLen;           //已发送长度  
    int nSedSize;               //发送数据大小  
    int nRcvBufLen;             //缓冲区长度  
    int nRcvSize;               //接收body数据大小  
  
    char cSndFilePathName[128]; //上送文件路径名  
    char *pRcvBuffer;           //指向用户用来存储数据的buf  
    char *pSendBuffer;          //指向用户发送数据的buf  
}stFtpUserInfo;  
  
typedef struct _stFtpClientInfo  
{  
    int nServerPort;            //ftp服务器监听端口号  
    char chServerIP[32];        //ftp服务器IP  
    char chServerPath[128];     //ftp服务器文件路径  
    char chUserName[32];        //ftp服务器登录用户  
    char chUserPassWord[32];    //ftp服务器登录密码  
  
    int use_proxy;              //是否使用http服务器代理  
    int nHttpServerPort;        //ftp服务器监听端口号  
    char chHttpServerIP[32];    //ftp服务器IP  
    char chHttpServerPath[64];//ftp服务器文件路径  
  
    stFtpUserInfo stFtpUser;  
}stFtpClientInfo;  
  
int ftpClient_ftpUpload(stFtpClientInfo *pstUserArg,long *pRespCode);  
  
#ifdef __cplusplus  
}  
#endif  

#endif
