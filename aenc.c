/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-05-29 12:41:49
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-06-16 17:04:23
 */ 
/*
 进行音频采集，采集pcm数据并直接保存pcm数据
 音频参数： 
	 声道数：		1
	 采样位数：	16bit、LE格式
	 采样频率：	44100Hz
运行示例:
$ gcc linux_pcm_save.c -lasound
$ ./a.out hw:0 123.pcm
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include "mp3_encoder/types.h"

config_t config;
int cutoff;
#define AudioFormat SND_PCM_FORMAT_S16_LE  //指定音频的格式,其他常用格式：SND_PCM_FORMAT_U24_LE、SND_PCM_FORMAT_U32_LE
#define AUDIO_CHANNEL_SET   2 			  //1单声道   2立体声
#define AUDIO_RATE_SET 44100  //音频采样率,常用的采样频率: 44100Hz 、16000HZ、8000HZ、48000HZ、22050HZ
 
FILE *pcm_data_file=NULL;
FILE *pcm_left_file=NULL;
int run_flag=0;
void exit_sighandler(int sig)
{
	run_flag=1;
}
 

 #define VOICE_PACAKGE_10MS 160000


const char* print_pcm_state(snd_pcm_state_t state)
{
    switch(state)
    {
    case SND_PCM_STATE_OPEN: return("SND_PCM_STATE_OPEN");
    case SND_PCM_STATE_SETUP: return("SND_PCM_STATE_SETUP");
    case SND_PCM_STATE_PREPARED: return("SND_PCM_STATE_PREPARED");
    case SND_PCM_STATE_RUNNING: return("SND_PCM_STATE_RUNNING");
    case SND_PCM_STATE_XRUN: return("SND_PCM_STATE_XRUN");
    case SND_PCM_STATE_DRAINING: return("SND_PCM_STATE_DRAINING");
    case SND_PCM_STATE_PAUSED: return("SND_PCM_STATE_PAUSED");
    case SND_PCM_STATE_SUSPENDED: return("SND_PCM_STATE_SUSPENDED");
    case SND_PCM_STATE_DISCONNECTED: return("SND_PCM_STATE_DISCONNECTED");
    }
return "UNKNOWN STATE";
}


int main(int argc, char *argv[])
{
	int i;
	int err;
	char *buffer,*buffer_left;
	int buffer_frames = 2048;
    uint32_t buffer_time, period_time;
	unsigned int rate = AUDIO_RATE_SET;
	snd_pcm_t *capture_handle;// 一个指向PCM设备的句柄
	snd_pcm_hw_params_t *hw_params; //此结构包含有关硬件的信息，可用于指定PCM流的配置
	
	/*注册信号捕获退出接口*/
	signal(2,exit_sighandler);
 
	/*PCM的采样格式在pcm.h文件里有定义*/
	snd_pcm_format_t format=AudioFormat; // 采样位数：16bit、LE格式
 
	/*打开音频采集卡硬件，并判断硬件是否打开成功，若打开失败则打印出错误提示*/
	// SND_PCM_STREAM_PLAYBACK 输出流
	// SND_PCM_STREAM_CAPTURE  输入流
	if ((err = snd_pcm_open (&capture_handle, "default",SND_PCM_STREAM_CAPTURE,0))<0) 
	{
		printf("无法打开音频设备: %s (%s)\n",  "deault",snd_strerror (err));
		exit(1);
	}
	printf("++++Sound card open Sucess....\n");
 
	/*创建一个保存PCM数据的文件*/
	if((pcm_data_file = fopen(argv[1], "wb")) == NULL)
	{
		printf("无法创建%s音频文件.\n",argv[1]);
		exit(1);
	} 


		/*创建一个保存PCM left数据的文件*/
	#if 0
	if((pcm_left_file = fopen(argv[2], "wb")) == NULL)
	{
		printf("无法创建%s音频文件.\n",argv[1]);
		exit(1);
	} 
	printf("record file:%s.is creator open sucess..\n",argv[1]);
	#endif
	/*分配硬件参数结构对象，并判断是否分配成功*/
	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) 
	{
		printf("无法分配硬件参数结构 (%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("++++hw params malloc sucess....\n");
	
	/*按照默认设置对硬件对象进行设置，并判断是否设置成功*/
	if((err=snd_pcm_hw_params_any(capture_handle,hw_params)) < 0) 
	{
		printf("无法初始化硬件参数结构 (%s)\n", snd_strerror(err));
		exit(1);
	}
	printf("++++hw param init sucess....\n");
 
	/*
		设置数据为交叉模式，并判断是否设置成功
		interleaved/non interleaved:交叉/非交叉模式。
		表示在多声道数据传输的过程中是采样交叉的模式还是非交叉的模式。
		对多声道数据，如果采样交叉模式，使用一块buffer即可，其中各声道的数据交叉传输；
		如果使用非交叉模式，需要为各声道分别分配一个buffer，各声道数据分别传输。
	*/
	if((err = snd_pcm_hw_params_set_access (capture_handle,hw_params,SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		printf("无法设置访问类型(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("++++acess type %d --interleaved set sucess.\n",SND_PCM_ACCESS_RW_NONINTERLEAVED);
 
	/*设置数据编码格式，并判断是否设置成功*/
	if ((err=snd_pcm_hw_params_set_format(capture_handle, hw_params,format)) < 0) 
	{
		printf("无法设置格式 (%s)\n",snd_strerror(err));
		exit(1);
	}
	fprintf(stdout, "+++ PCM Format[%d]S16_LE-%d set sucess.\n",AudioFormat,SND_PCM_FORMAT_S16_LE);
 
	/*设置采样频率，并判断是否设置成功*/
	if((err=snd_pcm_hw_params_set_rate_near(capture_handle,hw_params,&rate,0))<0) 
	{
		printf("无法设置采样率(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("++++ sample rate :%d set ok\n",rate);
 
	/*设置声道，并判断是否设置成功*/
	if((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params,AUDIO_CHANNEL_SET)) < 0) 
	{
		printf("无法设置声道数(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("+++audio nchannel:%d set ok.\n",AUDIO_CHANNEL_SET);
 
  #if 1
    if (snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max/n");
        exit(1);
    }
    printf("----Getbuffer timer...:%d\r\n",buffer_time);
    buffer_time = VOICE_PACAKGE_10MS;
    if (buffer_time > 500000)
    {
        buffer_time = 500000;
    }
    period_time = buffer_time / 4;

    if (snd_pcm_hw_params_set_buffer_time_near(capture_handle, hw_params, &buffer_time, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near/n");
        exit(1);
    }

    if (snd_pcm_hw_params_set_period_time_near(capture_handle, hw_params, &period_time, 0) < 0)
    {
		    fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near/n");
		   exit(1);
    }
    #endif





 
	/*将配置写入驱动程序中，并判断是否配置成功*/
	if ((err=snd_pcm_hw_params (capture_handle,hw_params))<0) 
	{
		printf("无法向驱动程序设置参数(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("+++++++pcm_hw_params++++++++++set over...OK\n");
 
	/*使采集卡处于空闲状态*/
	snd_pcm_hw_params_free(hw_params);
 
	/*准备音频接口,并判断是否准备好*/
	if((err=snd_pcm_prepare(capture_handle))<0) 
	{
		printf("无法使用音频接口 (%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("++++++Audio free--->Prepare ok.\n");
 
	/*配置一个数据缓冲区用来缓冲数据*/
	//snd_pcm_format_width(format) 获取样本格式对应的大小(单位是:bit)
	
    


    #if 1
    size_t frame_size;
    size_t buffer_size;
    snd_pcm_hw_params_get_period_size(hw_params, &frame_size, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    if (frame_size == buffer_size)
    {		
        fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)/n"), frame_size, buffer_size);		
        exit(1);
    }
     printf("frame_size (%lu == buffer_size %lu)\r\n", frame_size, buffer_size);		
    //获取样本长度
    int bits_per_sample = snd_pcm_format_physical_width(format);
    printf("bits_per_sample===%d\r\n",bits_per_sample);
    int bits_per_frame =  bits_per_sample;
    printf("bits_per_frame===%d\r\n",bits_per_frame);
    
    size_t chunk_bytes =  frame_size * bits_per_frame*AUDIO_CHANNEL_SET / 8;
    printf("chunk_bytes=====%d\r\n",chunk_bytes);

    int frame_byte = bits_per_frame*AUDIO_CHANNEL_SET / 8;
    int width = bits_per_sample/8;
    buffer_frames = frame_size;
    buffer=malloc(chunk_bytes); //2048
	if(AUDIO_CHANNEL_SET == 0x02)
	{
	  buffer_left = malloc(chunk_bytes/2);
	  
	}
	
    #else

    int width=snd_pcm_format_width(format);
     int frame_byte =width /8;
	buffer=malloc(buffer_frames*frame_byte*AUDIO_CHANNEL_SET); //2048
    #endif
   
	printf("+++++++++++Frame byte%d width<%d>=[(format<%d>)/8],record buffer malloc OK.\n",frame_byte,width,format);
    printf("+++buffersize =<%d>==buffer_frames<%d>*frame_byte<%d>*2++++\r\n",buffer_frames*frame_byte*AUDIO_CHANNEL_SET,buffer_frames,frame_byte);
	
	/*开始采集音频pcm数据*/
	printf("++++start...capture Audio..frame_byte:%d,frame_size:%d,++++.\n",frame_byte,frame_size);






	while(1) 
	{
        #if 1
		/*从声卡设备读取一帧音频数据:2048字节*/

	   snd_pcm_state_t pcm_state = snd_pcm_state(capture_handle);  
        //printf(" State: %s\n", print_pcm_state(pcm_state));
		if((err=snd_pcm_readi(capture_handle,buffer,frame_size))!=frame_size) 
		{
			  printf("从音频接口读取失败(%s),\n",snd_strerror(err));
			  exit(1);
		}
        #else
        /*从声卡设备读取一帧音频数据:2048字节*/
                if((err=snd_pcm_readi(capture_handle,buffer,buffer_frames))!=buffer_frames) 
                {
                    printf("从音频接口读取失败(%s)\n",snd_strerror(err));
                    exit(1);
                }

        #endif
		/*写数据到文件: 音频的每帧数据样本大小是16位=2个字节*/
        #if 1
		if(AUDIO_CHANNEL_SET == 0x02)
		{
		
		#if 0
			unsigned short *left_ptr = (unsigned short *)buffer_left;
			unsigned short *all_ptr  = (unsigned short *)buffer;
			int j =0;
			
	      for(i = 0; i< frame_size*2 ;i++)
		  {
			  //printf("i=%d,j=%d\r\n",i,j);
			  if(i % 2 == 0)
			  {
				left_ptr[j++] = all_ptr[i];
			  }
		  }
		  //printf("j===%d,i==%d,frame_size:%d frame_byte:%d\n",j,i,frame_size,frame_byte/2);
		   fwrite(buffer_left,frame_size,frame_byte/2,pcm_left_file);
		   #endif
		   fwrite(buffer,frame_size,frame_byte,pcm_data_file);	
		}else
		{
		   fwrite(buffer,frame_size,frame_byte,pcm_data_file);	
		}
		
		
        #else
        fwrite(buffer,(buffer_frames*AUDIO_CHANNEL_SET),bits_per_frame,pcm_data_file);	
        #endif
		if(run_flag)
		{
			printf("+++++stop audio.+++++\n");
			break;
		}
	}
 
	/*释放数据缓冲区*/
	free(buffer);
 
	/*关闭音频采集卡硬件*/
	snd_pcm_close(capture_handle);
 
	/*关闭文件流*/
	fclose(pcm_data_file);
	return 0;
}