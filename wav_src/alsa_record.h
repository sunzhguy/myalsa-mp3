/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-05-29 11:26:52
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-05-29 11:38:14
 */ 

#ifndef COM_HXIONG_ALSA_RECORD_H
#define COM_HXIONG_ALSA_RECORD_H
 
//宏定义
#define RECORD_FAIL   -1
#define RECORD_SUCCESS 0
#define RECORD_SAMPLE_RATE      8000
#define RECORD_BUFFER_TIME_MAX  500000
#define RECORD_DEVICE_NAME      "default"
 
 
//wav 格式的文件头
typedef struct{
    char chunk_id[4];   /* "RIFF" */
    char chunk_size[4]; /* file total size - 8(byte) */
    char format[4];    /* "WAVE" */
    /* sub-chunk "fmt" */
    char sub_chunk_id[4];   /* "fmt " */
    char sub_chunk_size[4]; /* 16 for PCM */
    char audio_format[2];   /* PCM = 1*/
    char audio_channels[2];   /* Mono = 1, Stereo = 2, etc. */
    char samplerate[4];    /* 8000, 44100, etc. */
    char byterate[4];  /* = SampleRate * NumChannels * BitsPerSample/8 */
    char block_align[2];    /* = NumChannels * BitsPerSample/8 */
    char bits_persample[2]; /* 8bits, 16bits, etc. */
    //char fmt_chunk_end[2];  /* 附加的两个数据，值为0 */
 
    /* sub-chunk "data" */
    char sub_chunk2_id[4];   /* "data" */
    char sub_chunk2_size[4]; /* data size */
} wav_format_t;
 
 
 
//自定义的结构体参数，目的时为了把command 中的参数最一层转换，到do_alsa_record 会再转成record_handle_t 中参数
typedef struct{
        int duration;  //录制的时长
        int format;    //录制的pcm格式
        int rate;      //采样率
        int channel;   //channel 个数
        // 如有需要其他参数可以继续添加
} record_params_t;
//path 为录制的wav音频文件的保存路径
int do_alsa_record(char* path,record_params_t* params);

#endif