/* wave.c
 *
 * Data input functions.
 */

#include "types.h"
#include "../ring_buffer/kring_buffer.h"

/*
 * wave_close:
 * -----------
 */
void wave_close(void)
{
  fclose(config.wave.file);
}

/*
 * wave_open:
 * ----------
 * Opens and verifies the header of the Input Wave file. The file pointer is
 * left pointing to the start of the samples.
 */
void wave_open(int raw, int mono_from_stereo)
{

  static char *channel_mappings[] = {NULL,"mono","stereo"};
  static char *file_type[] = {"RIFF-WAVE PCM","RAW DATA PCM"};

  /* Wave file headers can vary from this, but we're only intereseted in this format */
  struct wave_header
  {
    char riff[4];             /* "RIFF" */
    unsigned long size;       /* length of rest of file = size of rest of header(36) + data length */
    char wave[4];             /* "WAVE" */
    char fmt[4];              /* "fmt " */
    unsigned long fmt_len;    /* length of rest of fmt chunk = 16 */
    unsigned short tag;       /* MS PCM = 1 */
    unsigned short channels;  /* channels, mono = 1, stereo = 2 */
    unsigned long samp_rate;  /* samples per second = 44100 */
    unsigned long byte_rate;  /* bytes per second = samp_rate * byte_samp = 176400 */
    unsigned short byte_samp; /* block align (bytes per sample) = channels * bits_per_sample / 8 = 4 */
    unsigned short bit_samp;  /* bits per sample = 16 for MS PCM (format specific) */
    char data[4];             /* "data" */
    unsigned long length;     /* data length (bytes) */
  } header;

  /* open input file */
  if((config.wave.file = fopen(config.infile,"rb")) == NULL)
    printf("Unable to open file\r\n");

  if(raw)
  {
    #if 0
      fseek(config.wave.file, 0, SEEK_END); /* find length of file */
      header.length = ftell(config.wave.file);
      fseek(config.wave.file, 0, SEEK_SET);
     #endif
      header.channels = (mono_from_stereo) ? 1 : 2;
      header.samp_rate = config.wave.samplerate; /* 44100 if not set by user */
      header.bit_samp = 16;     /* assumed */
      header.byte_samp = header.channels * header.bit_samp / 8;
      header.byte_rate = header.samp_rate * header.byte_samp;
  }
  else /* wave */
  {
      if (fread(&header, sizeof(header), 1, config.wave.file) != 1)
        error("Invalid Header");
      if(strncmp(header.riff,"RIFF",4) != 0) error("Not a MS-RIFF file");
      if(strncmp(header.wave,"WAVE",4) != 0) error("Not a WAVE audio");
      if(strncmp(header.fmt, "fmt ",4) != 0) error("Can't find format chunk");
      if(header.tag != 1)                    error("Unknown WAVE format");
      if(header.channels > 2)                error("More than 2 channels");
      if(header.bit_samp != 16)              error("Not 16 bit");
      if(strncmp(header.data,"data",4) != 0) error("Can't find data chunk");
  }

  config.wave.channels      = header.channels;
  config.wave.samplerate    = header.samp_rate;
  config.wave.bits          = header.bit_samp;
  config.wave.total_samples = header.length / header.byte_samp;
  config.wave.length        = header.length / header.byte_rate;

  if((config.wave.channels == 1) || mono_from_stereo)
  {
    config.mpeg.channels = 1;
    config.mpeg.mode = MODE_MONO;
  }

  printf("%s, %s %ldHz %dbit, Length: %2ld:%2ld:%2ld\n",
          file_type[raw],
          channel_mappings[config.wave.channels],
          config.wave.samplerate,
          config.wave.bits,
          config.wave.length/3600,(config.wave.length/60)%60,config.wave.length%60);
}

/*
 * wave_get:
 * ---------
 * Reads samples from the file in longs. A long can
 * hold one stereo or two mono samples.
 * Returns the address of the start of the next frame or NULL if there are no
 * more. The last frame will be padded with zero's.
 */
unsigned long int *wave_get(void)
{
  int n,p;
  static unsigned long buff[samp_per_frame];

  n = config.mpeg.samples_per_frame >> (2 - config.wave.channels);

  p = fread(buff,sizeof(unsigned long),(short)n,config.wave.file);
  if(!p)
    return 0;
  else
  {
    for(; p<n; p++)
      buff[p]=0;
    return buff;
  }
}

/*
 * wave_get_bysunzhguy:
 * ---------
 * Reads samples from the file in longs. A long can
 * hold one stereo or two mono samples.
 * Returns the address of the start of the next frame or NULL if there are no
 * more. The last frame will be padded with zero's.
 */
static time_t rcd_time = 0; /*时间 临时变量=time(NULL)*/
#define  ENCODER_ON   1
#define  ENCODER_OFF  0
int encode_status = ENCODER_ON;
extern struct  kring_buffer * ring_buf ;

static void msleep(int ms)
{

	usleep(1000*ms);

}

void set_mp3encoder_off(void)
{

  encode_status = ENCODER_OFF;
}

extern int run_flag ;
void init_time(void)
{
  	rcd_time = time(NULL);    /*record init time*/
}
unsigned long int *wave_get_bysunzhguy(void)
{
  int n,p;
  static unsigned long buff[samp_per_frame];
  static unsigned short i = 1;
  time_t tmp_t = 0;
   if(0 == i%28)    /*28 = 1S*/
    {
        tmp_t = time(NULL);
        if((tmp_t - rcd_time) > 3600)
        {
            encode_status = ENCODER_OFF;
            i = 1;
            printf("record times reach 30 mintues ,stop record\n");    //hyh
        }
    
    }
    if(run_flag)
    encode_status = ENCODER_OFF;

    i++;
    if(encode_status == ENCODER_OFF)
    {
      i = 1;
      return 0;
    }

  n = config.mpeg.samples_per_frame >> (2 - config.wave.channels);
  
 if(kring_buffer_len(ring_buf) >= sizeof(unsigned long)*n)
 {
   p = kring_buffer_get(ring_buf, ((u8*)(buff)),sizeof(unsigned long)*n);
 }else
 {
   printf("get_len:%ld,n==%d,%d,%d\r\n",kring_buffer_len(ring_buf),n, sizeof(unsigned long)*n,sizeof(unsigned long));
   p = 0;
 }
 

	while(p <= 0)
	{
		msleep(1);
    if(kring_buffer_len(ring_buf) <sizeof(unsigned long)*n)
    {
      	msleep(1);
    }

		if(encode_status == ENCODER_ON)
		{
      if(kring_buffer_len(ring_buf) >=sizeof(unsigned long)*n)
      {
        p = kring_buffer_get(ring_buf, ((u8*)(buff)),sizeof(unsigned long)*n);
        printf("+++++++++p====%d\r\n",p);
      }else
      {
        p =0;
      }
      
		
		}
		else
		{
			printf("Layer3 go out,,read none \n");
		
			return 0;

		}
	}
    p = p/sizeof(unsigned long);

  //p = fread(buff,sizeof(unsigned long),(short)n,config.wave.file);
  if(!p)
    return 0;
  else
  {
    for(; p<n; p++)
      buff[p]=0;
    return buff;
  }
}