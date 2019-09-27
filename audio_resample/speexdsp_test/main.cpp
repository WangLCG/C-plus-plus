#include <stdio.h>
#include <iostream>
#include <memory>
#include <string.h>

#include<speex/speex_resampler.h>
using namespace std;


int main()
{
    SpeexResamplerState *resampler = NULL;
    int err = 0;

    /* 48khz ---> 8khz */
    resampler = speex_resampler_init(2,48000,8000,6,&err);
    speex_resampler_set_rate(resampler, 48000, 8000);
    
    FILE* pcmfd = fopen ("test.pcm", "rb+");
    if(!pcmfd)
    {
        //cout << "open test.cpm faile" << endl;
        printf("open test.cpm faile \n");
        return -1;
    }

    FILE* outfd = fopen("8k.pcm", "wb+");
    if(!outfd)
    {
        fclose(pcmfd);
        printf("open test.cpm faile \n");
        return -1;
    }
    
    int samples = 2048;
    int channel = 2;
    
    int buffer_size = samples * channel;
    char* pcmbuffer = new char[buffer_size];
    memset(pcmbuffer, 0, buffer_size);

    unsigned char* out_buffer = new unsigned char[buffer_size*2];
    memset(out_buffer, 0, buffer_size*2);
    
    unsigned char* last_buffer = new unsigned char[buffer_size];
    memset(last_buffer, 0, buffer_size);
    
    int frame = 0;
    int ret   = 0;

    
    while(fread(pcmbuffer, buffer_size,1 ,pcmfd) != 0)
    {
        memset(out_buffer, 0, buffer_size*2);
        memset(last_buffer, 0, buffer_size);
        
        unsigned int inlen  = buffer_size/2 ;
        unsigned int outlen = buffer_size ;
        
        ret = speex_resampler_process_interleaved_int(resampler, (short*)pcmbuffer,&inlen, (short*)out_buffer, &outlen );
        if(ret == RESAMPLER_ERR_SUCCESS)
        {
            int i = 0 , j = 0;
            for(; i< outlen*2; i+=4,j+=2)
            {
                last_buffer[j]   = out_buffer[i];
                last_buffer[j+1] = out_buffer[i+1];
            }
            //fwrite(out_buffer, sizeof(short), outlen, outfd);  /* 输出双声道 */
            fwrite(last_buffer, sizeof(short), outlen/2, outfd);
            
            
            printf("processed[%d] outlen = %d \n", inlen, outlen);
        }
        else
        {
            printf("error: %d\n", ret);
        }
    }

    speex_resampler_destroy(resampler);
    
    delete[] out_buffer;
    delete[] last_buffer;
    delete[] pcmbuffer;
    
    fclose(pcmfd);
    fclose(outfd);
    
    return 0;
}

