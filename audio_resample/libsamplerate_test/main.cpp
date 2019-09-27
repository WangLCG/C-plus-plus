#include <samplerate.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <string.h>

using namespace std;

int process_test();

int main()
{
    SRC_DATA m_DataResample;

    float in[4096]={0};
    float out[4096]={0};
    
    int samples = 2048;
    int channel = 2;

    m_DataResample.data_in       = in;
    m_DataResample.input_frames  = samples ;
    m_DataResample.data_out      = out;
    m_DataResample.output_frames = samples ;
    m_DataResample.src_ratio     = 8000.0/48000;  /* 输出采样率/输入采样率 */

    
    int RetResample = 0;

    FILE* pcmfd = fopen ("test.pcm", "rb+");
    if(!pcmfd)
    {
        //cout << "open test.cpm faile" << endl;
        printf("open test.cpm faile \n");
        return -1;
    }

    FILE* outfd = fopen("8k.pcm","wb+");
    if(!outfd)
    {
        printf("open 8k.cpm faile \n");
        fclose(pcmfd);
        return -1;
    }

    int buffer_size = samples * channel;
    char* pcmbuffer = new char[buffer_size];
    memset(pcmbuffer, 0, buffer_size);

    unsigned char* out_buffer = new unsigned char[buffer_size];
    memset(out_buffer, 0, buffer_size);

    int frame = 0;
    while(fread(pcmbuffer, buffer_size,1 ,pcmfd) != 0)
    {
        if(frame % 10 == 0)
            printf("processing the %d frame \n", frame);

        memcpy(out_buffer, pcmbuffer, buffer_size);

        memset(in , 0 , 4096 * sizeof(float));
        memset(out , 0 , 4096 * sizeof(float));
        
        for(int j = 0; j < 4096 && j < buffer_size; j++)
        {
            in[j] = pcmbuffer[j];
        }

        RetResample = src_simple(&m_DataResample, SRC_LINEAR, 2);
        if(RetResample != 0)
        {
            printf("src_simple error: %s", src_strerror(RetResample));
            break;
        }

        printf("input_frame_used[%d], out_frame_gen[%d]\n", m_DataResample.input_frames_used , m_DataResample.output_frames_gen);

        int i = 0, j = 0;
        int buf_sizePCM = m_DataResample.output_frames_gen * channel;
    
        for (; i < 4096 && i < buf_sizePCM && j<2048; i += 4, j += 2) 
        {
            out_buffer[j]     = (unsigned char)(out[i]);
            out_buffer[j + 1] = (unsigned char)(out[i + 1]);
        }
        buf_sizePCM = buf_sizePCM / 2;

        if(frame == 2)
            printf("i = %d, j = %d \n", i, j);

        frame++;

        fwrite(out_buffer, 1, buf_sizePCM, outfd);
        fflush(outfd);

        memset(out_buffer, 0, buffer_size);
        memset(pcmbuffer, 0, buffer_size);
    }

    delete[] pcmbuffer;

    fclose(pcmfd);
    fclose(outfd);


    /* src_process处理流程 */
    process_test();

    return 0;
    
}

int process_test()
{
    SRC_DATA m_DataResample;

    SRC_STATE* src_state = NULL;
    int error = -1;
    src_state = src_new(SRC_LINEAR, 2, &error);
    if(!src_state)
    {
        printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
        return -1;
    }
    
    float in[4096]={0};
    float out[4096]={0};

    
    int samples = 2048;
    int channel = 2;

    m_DataResample.data_in       = in;
    m_DataResample.input_frames  = samples ;
    m_DataResample.data_out      = out;
    m_DataResample.output_frames = samples ;
    m_DataResample.src_ratio     = 8000.0/48000;  /* 输出采样率/输入采样率 */

    
    int RetResample = 0;

    FILE* pcmfd = fopen ("test.pcm", "rb+");
    if(!pcmfd)
    {
        //cout << "open test.cpm faile" << endl;
        printf("open test.cpm faile \n");
        return -1;
    }

    FILE* outfd_process = fopen("8k_process.pcm","wb+");
    if(!outfd_process)
    {
        printf("open 8k_process.cpm faile \n");
        fclose(pcmfd);
        return -1;
    }

    int buffer_size = samples * channel;
    char* pcmbuffer = new char[buffer_size];
    memset(pcmbuffer, 0, buffer_size);

    unsigned char* out_buffer = new unsigned char[buffer_size];
    memset(out_buffer, 0, buffer_size);

    int frame = 0;
    while(fread(pcmbuffer, buffer_size,1 ,pcmfd) != 0)
    {
        if(frame % 10 == 0)
            printf("processing the %d frame \n", frame);

        //memcpy(out_buffer, pcmbuffer, buffer_size);

        memset(in , 0 , 4096 * sizeof(float));
        memset(out , 0 , 4096 * sizeof(float));
        
        for(int j = 0; j < 4096 && j < buffer_size; j++)
        {
            in[j] = pcmbuffer[j];
        }

        /* SRC_PROCESS 处理流程 */
        m_DataResample.end_of_input  = 1;
        m_DataResample.data_in       = in;
        m_DataResample.input_frames  = samples ;
        m_DataResample.data_out      = out;
        m_DataResample.output_frames = samples ;
        m_DataResample.src_ratio     = 8000.0/48000;  /* 输出采样率/输入采样率 */
        
        //while(1)
        {
            src_reset(src_state);
            int ret = src_process(src_state, &m_DataResample);
            if(0 == ret)
            {
                int buf_sizePCM = m_DataResample.output_frames_gen * channel;
                int i = 0, j = 0;
                for (; i < 4096 && i < buf_sizePCM && j<2048; i += 4, j += 2) 
                {
                    out_buffer[j]     = (unsigned char)(out[i]);
                    out_buffer[j + 1] = (unsigned char)(out[i + 1]);
                }
                buf_sizePCM = buf_sizePCM / 2;

                fwrite(out_buffer, 1, buf_sizePCM, outfd_process);
                fflush(outfd_process);
                
                memset(out_buffer, 0, buffer_size);
                memset(out , 0 , 4096 * sizeof(float));
                printf("-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d] src_ratio[%f]------ \n", 
                    m_DataResample.output_frames_gen, m_DataResample.input_frames_used, 
                    m_DataResample.end_of_input, m_DataResample.src_ratio);
            }
            else
            {
                printf("src_simple error: %s \n", src_strerror(ret));
                printf("111-------- output_frames_gen[%d], in_used_frame[%d] end_of_input[%d]------\n", 
                    m_DataResample.output_frames_gen, m_DataResample.input_frames_used, m_DataResample.end_of_input);
                //break;
            }

            //if(m_DataResample.input_frames_used == samples)
            //{
                //m_DataResample.end_of_input = 1;
                //break;
            //}
            
           // m_DataResample.data_in          += m_DataResample.input_frames_used;
           // m_DataResample.input_frames     -= m_DataResample.input_frames_used;
           // m_DataResample.output_frames_gen = 0;
            
        }

        frame++;
        memset(out_buffer, 0, buffer_size);
        memset(pcmbuffer, 0, buffer_size);
    }

    delete[] pcmbuffer;
    src_delete (src_state) ;
    fclose(pcmfd);
    fclose(outfd_process);
    
    return 0;
    
}



