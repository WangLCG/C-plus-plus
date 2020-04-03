#ifdef __cplusplus  
    extern "C" {  
#endif  
      
  
/***************************************************************** 
* 包含头文件 
******************************************************************/  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <curl/curl.h>
#include "ftpclient.h"  
  
  
/************************************************* 
** Function：ftpClient_writeCallBack 
** Description：ftp回调数据接收函数 
** Input:无 
** Output： 
 
** Return：-1-失败 0-成功 
** Author： 
** Date：2018-11-27 
 
** Modification History： 
** Author： 
** Date： 
** Description： 
*************************************************/  
size_t ftpClient_writeCallBack(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
    int writeLength = 0;  
    int written  = size*nmemb;  
    stFtpUserInfo *pSt = (stFtpUserInfo*)stream;      
  
    if(0)//此处记录body内容  
    {  
        if(pSt->pRcvBuffer != NULL)  
        {  
            writeLength = (written <= (pSt->nRcvBufLen - pSt->nRcvSize))?(written):(pSt->nRcvBufLen - pSt->nRcvSize);  
            memcpy((char*)(pSt->pRcvBuffer) +pSt->nRcvSize, ptr, writeLength);  
            pSt->nRcvSize += writeLength;  
            //printf("%d line,nWRLength=%d written=%d nUsedLength=%d writeLength=%d\n",__LINE__,pSt->nRcvBufLen, written,pSt->nRcvSize,writeLength);  
        }  
        return writeLength;  
    }  
    if(0){  
        printf("\n %s\n", ptr);  
    }  
    return 0;  
}  
  
/************************************************* 
** Function：ftpClient_file2Ftp 
** Description：ftp读取文件并上送 
** Input:无 
** Output： 
 
** Return：-1-失败 0-成功 
** Author： 
** Date：2018-11-27 
 
** Modification History： 
** Author： 
** Date： 
** Description： 
*************************************************/  
size_t ftpClient_file2Ftp(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
    size_t retcode = fread(ptr, size, nmemb, stream);  
    return retcode;  
}  
  
  
/************************************************* 
** Function：ftpClient_mem2Ftp 
** Description：ftp读取内存数据 
** Input:无 
** Output： 
 
** Return：-1-失败 0-成功 
** Author： 
** Date：2018-11-27 
 
** Modification History： 
** Author： 
** Date： 
** Description： 
*************************************************/  
size_t ftpClient_mem2Ftp(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
    size_t cpoyLength = 0;
    size_t copySize  = size*nmemb;
    stFtpUserInfo *pSt = (stFtpUserInfo*)stream;
      
    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1))
    {
        return 0;
    }
      
    if(pSt->pSendBuffer != NULL)
    {  
        cpoyLength = (copySize <= (pSt->nSedSize - pSt->nHaveSendLen))?(copySize):(pSt->nSedSize - pSt->nHaveSendLen);
        memcpy(ptr,(char*)(pSt->pSendBuffer) +pSt->nHaveSendLen, cpoyLength);
        pSt->nHaveSendLen += cpoyLength;
        //printf("%d line,copySize=%d nSedSize=%d nHaveSendLen=%d cpoyLength=%d\n",__LINE__,copySize, pSt->nSedSize,pSt->nHaveSendLen,cpoyLength);  
    }

    /* 销毁 */
    if(pSt->nHaveSendLen == pSt->nSedSize)
    {
        delete[] pSt->pSendBuffer;
        pSt->pSendBuffer = NULL;
    }

    return cpoyLength;
}  
  
  
/************************************************* 
** Function：ftpClient_ftpUpload 
** Description：ftpUpload 
** Input:无 
** Output： 
 
** Return：-1-失败 0-成功 
** Author： 
** Date：2018-11-27 
 
** Modification History： 
** Author： 
** Date： 
** Description： 
*************************************************/  
int ftpClient_ftpUpload(stFtpClientInfo *pstUserArg,long *pRespCode)  
{  
    int nRet = 0;  
    long response_code      = -1;  
    char chftpUrl[160]  ={0};  
    char chHttpUrl[128]     ={0};  
  
    FILE *ffp = NULL;  
    curl_off_t fsize = 0;  
  
    CURL *curl = NULL;  
    CURLcode code = 0;  
    struct stat file_info;  
    struct curl_slist *chunk = NULL;      
  
    *pRespCode = 0;  
  
    if(NULL == pstUserArg){  
        printf("%s func Invalid parameter at (%d) line\n",__FUNCTION__,__LINE__);  
        return -1;  
    }  
  
    curl = curl_easy_init();  
    if(curl)  
    {  
        snprintf(chftpUrl,160,"ftp://%s:%u/%s", pstUserArg->chServerIP, pstUserArg->nServerPort, pstUserArg->chServerPath);
          
    //  printf("%s func User=%s ,passwd=%s,chUrl=%s at (%d) line\n",__FUNCTION__,pstUserArg->chUserName,pstUserArg->chUserPassWord,chftpUrl,__LINE__);  
 
  		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
        curl_easy_setopt(curl, CURLOPT_URL, chftpUrl);  
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L); //上传  
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);   //注意，毫秒超时一定要设置这个  
        curl_easy_setopt(curl, CURLOPT_USERNAME, pstUserArg->chUserName);  
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pstUserArg->chUserPassWord);          
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 88000L);    //设置延时1s  
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);//创建不存在的目录  
          
        /* enable TCP keep-alive for this transfer */  
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);  
        /* keep-alive idle time to 120 seconds */  
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 600L);  
        /* interval time between keep-alive probes: 60 seconds */  
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);  
  
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ftpClient_writeCallBack);  
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pstUserArg->stFtpUser);   /*接收响应*/  
  
        switch(pstUserArg->stFtpUser.nOperation)
        {  
            case FTP_OPERATION_UPLOAD_MEM:  //发送内存数据  
                curl_easy_setopt (curl, CURLOPT_READDATA, &pstUserArg->stFtpUser);                 
                curl_easy_setopt (curl, CURLOPT_READFUNCTION, ftpClient_mem2Ftp);  
                curl_easy_setopt (curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) pstUserArg->stFtpUser.nSedSize);  
                break;  
            case FTP_OPERATION_UPLOAD_FILE:  //ftp发送文件  
  
                /********************打开文件***************/  
                if(stat(pstUserArg->stFtpUser.cSndFilePathName, &file_info)){  
                    printf("ftp upload stat %s failed\n", pstUserArg->stFtpUser.cSndFilePathName);  
                    goto iExit;  
                }  
                fsize = (curl_off_t) file_info.st_size;  
                ffp = fopen (pstUserArg->stFtpUser.cSndFilePathName, "rb");  
                if(NULL == ffp){  
                    printf("ftp fopen %s failed!\n", pstUserArg->stFtpUser.cSndFilePathName);  
                    goto iExit;  
                }  
  
                curl_easy_setopt (curl, CURLOPT_READDATA, ffp);               
                curl_easy_setopt (curl, CURLOPT_READFUNCTION, ftpClient_file2Ftp);  
                curl_easy_setopt (curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) fsize);  
                break;  
            default:  
                break;  
        }  
  
        /*是否使用了http代理服务器*/  
        if(pstUserArg->use_proxy == 1)  
        {  
            snprintf(chHttpUrl,128,"http://%s/%s/%s", pstUserArg->chHttpServerIP, pstUserArg->nHttpServerPort, pstUserArg->chHttpServerPath);  
            curl_easy_setopt (curl, CURLOPT_PROXY, chHttpUrl);  
        }  
  
        code = curl_easy_perform(curl);  
        if(code != CURLE_OK)  
        {  
            nRet = -1;  
            printf("%s func curl_easy_perform failed:%s at (%d) line\n",__FUNCTION__,curl_easy_strerror(code),__LINE__);  
        }   
        else  
        {  
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);  
            if(226 != response_code)//对于ftp成功是226码  
            {  
                nRet = -1;  
                printf("%s func curl_easy_getinfo RESPONSE_CODE: %d at (%d) line\n",__FUNCTION__,response_code,__LINE__);  
            }  
        }         
          
        switch(pstUserArg->stFtpUser.nOperation)  
        {  
            case FTP_OPERATION_UPLOAD_MEM:  //发送内存数据  
                break;  
            case FTP_OPERATION_UPLOAD_FILE:  //ftp发送文件  
                if(NULL != ffp) fclose(ffp);  
                break;  
            default:  
                break;  
        }  
  
        curl_slist_free_all(chunk);  
        curl_easy_cleanup(curl);  
          
        *pRespCode = response_code;  
        return nRet;  
    }  
      
    return -1;  
  
iExit:  
    curl_slist_free_all(chunk);  
    curl_easy_cleanup(curl);  
    return -1;  
}  
  
  
#ifdef __cplusplus  
}
#endif
