/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-06-05 15:34:51
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-06-16 18:16:58
 */ 

#ifndef  KRING_FIFO_BUFFER_H
#define  KRING_FIFO_BUFFER_H

typedef  unsigned  int uint32_t;
typedef  unsigned  char u8;
struct  kring_buffer
{
     void *   fbuffer;//fifo缓冲区
     uint32_t size  ;//大小
     uint32_t in;    //input 
     uint32_t out;   //output
     pthread_mutex_t *f_lock;//fifo互斥锁
};




struct kring_buffer * kring_bufer_alloc_init(uint32_t size);
void  kring_bufer_alloc_exit(struct kring_buffer *kring_buf);
uint32_t kring_buffer_len(const struct kring_buffer *kring_buf);
uint32_t kring_buffer_put(struct kring_buffer *kring_buf,void * fbuffer, uint32_t size);
uint32_t kring_buffer_get (struct kring_buffer *kring_buf,void * fbuffer,uint32_t size);

#endif