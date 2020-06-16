/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-06-05 15:35:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-06-08 15:42:21
 * 参考连接:https://www.cnblogs.com/java-ssl-xy/p/7868531.html
 * http://en.wikipedia.org/wiki/Circular_buffer
 */ 
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "kring_buffer.h"
/*判断X 是否2 的次方*/
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) -1))==0))

/*判断a, b 取最小值*/
#define min(a,b)   (((a) < (b)) ? (a) :(b))




static struct  kring_buffer * kring_buffer_init (void *fbuffer, uint32_t size, pthread_mutex_t *f_lock)
{
    assert(fbuffer);
    struct  kring_buffer *kring_buf = NULL;

    if(!is_power_of_2(size))
    {
        fprintf(stderr,"size must be powr of 2.\r\n");
        return kring_buf;
    }
    kring_buf = (struct  kring_buffer*)malloc(sizeof (struct  kring_buffer));
    if(!kring_buf)
    {
        fprintf(stderr,"Faile to malloc kring buffer %u,reasson:%s\r\n",errno,strerror(errno));
        return kring_buf;
    }
    memset(kring_buf, 0 ,sizeof(struct  kring_buffer));
    kring_buf->fbuffer = fbuffer;
    kring_buf->size = size ;
    kring_buf->in = 0;
    kring_buf->out = 0;
    kring_buf->f_lock = f_lock;
    return kring_buf;
}


/*释放kring_buffer*/
static void kring_buffer_free(struct  kring_buffer *kring_buf)
{
    if(kring_buf)
    {
        if(kring_buf->fbuffer)
        {
            free(kring_buf->fbuffer);
            kring_buf->fbuffer = NULL;
        }
        free(kring_buf);
        kring_buf = NULL;
    }
}

/*缓冲区长度*/
static uint32_t __kring_buffer_len(const struct kring_buffer *kring_buf)
{
   // printf("kring_buf->in:%d,kring_buf->out:%d\r\n",kring_buf->in,kring_buf->out);
    return (kring_buf->in - kring_buf->out);

}

/*获取数据从kring buffer*/
static uint32_t __kring_buffer_get(struct kring_buffer *kring_buf,void * fbuffer,uint32_t size)
{
   assert (kring_buf || fbuffer);
   uint32_t len = 0;
   size = min(size,kring_buf->in - kring_buf->out);
   /*first get the data form fifo->out unsil the end of the buffer*/
   len = min(size,kring_buf->size - (kring_buf->out & (kring_buf->size -1)));
   memcpy(fbuffer,kring_buf->fbuffer+(kring_buf->out &(kring_buf->size -1)),len);
   /*then get the rest (if any) from the beginning of the buffer*/
   memcpy(fbuffer+len,kring_buf->fbuffer,size-len);
   kring_buf->out += size;
   return size;
}
//向缓冲区中存放数据
static uint32_t __kring_buffer_put(struct kring_buffer *kring_buf,void * fbuffer,uint32_t size)
{
    assert(kring_buf || fbuffer);
    uint32_t len = 0;
    size = min(size, kring_buf->size - kring_buf->in + kring_buf->out);
     /* first put the data starting from fifo->in to buffer end */
    len  = min(size, kring_buf->size - (kring_buf->in & (kring_buf->size - 1)));
     memcpy(kring_buf->fbuffer + (kring_buf->in & (kring_buf->size - 1)),fbuffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(kring_buf->fbuffer, fbuffer + len, size - len);
    kring_buf->in += size;
   return size;

}

uint32_t kring_buffer_len(const struct kring_buffer *kring_buf)
{
      uint32_t len = 0;
      pthread_mutex_lock(kring_buf->f_lock);
      len = __kring_buffer_len(kring_buf);
      pthread_mutex_unlock(kring_buf->f_lock);
      return len;
}


uint32_t kring_buffer_get (struct kring_buffer *kring_buf,void * fbuffer,uint32_t size)
{
    uint32_t ret;
    pthread_mutex_lock(kring_buf->f_lock);
    ret = __kring_buffer_get(kring_buf,fbuffer,size);
    if(kring_buf->in == kring_buf->out)
    kring_buf->in = kring_buf->out = 0;
    pthread_mutex_unlock(kring_buf->f_lock);
    return ret;
}


uint32_t kring_buffer_put(struct kring_buffer *kring_buf,void * fbuffer, uint32_t size)
{
     uint32_t ret;
     pthread_mutex_lock(kring_buf->f_lock);
     ret = __kring_buffer_put(kring_buf,fbuffer,size);
     pthread_mutex_unlock(kring_buf->f_lock);
     return ret;
}


pthread_mutex_t *f_mutex_lock = NULL; /*需要互斥锁*/

struct kring_buffer * kring_bufer_alloc_init(uint32_t size)
{
    void * buffer = NULL;
    struct kring_buffer *kring_buf =NULL;
    pthread_mutex_t *f_mutex_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    if (pthread_mutex_init(f_mutex_lock, NULL) != 0)
    {
        fprintf(stderr, "Failed init mutex,errno:%u,reason:%s\n",errno, strerror(errno));
        goto err;
    }
    buffer = (void *)malloc(size);
    if (!buffer)
    {
      fprintf(stderr, "Failed to malloc memory.\n");
      goto err_buf;
    }
    kring_buf = kring_buffer_init(buffer, size, f_mutex_lock);
    if(kring_buf == NULL)
    goto err;
    
    return kring_buf;
err:
    free(buffer);
 err_buf:
    free(f_mutex_lock);
 err_lock:
    return NULL;
}
void  kring_bufer_alloc_exit(struct kring_buffer *kring_buf)
{
    kring_buffer_free(kring_buf);
    if(f_mutex_lock)
    free(f_mutex_lock);
}



#if 0
int main()
{
    char hello[10]={1,2,3,4,5,6,7,8,9,10};
    int len =0;
     struct kring_buffer *kring_buf =NULL;
     kring_buf=kring_bufer_alloc_init(8);
     if(kring_buf)
     {
         len =kring_buffer_len(kring_buf);
         printf("len ==%d\r\n",len);
         kring_buffer_put(kring_buf,hello,2);
         len =kring_buffer_len(kring_buf);
         printf("len ==%d\r\n",len);
         kring_buffer_put(kring_buf,hello,8);
         len =kring_buffer_len(kring_buf);
         printf("len ==%d\r\n",len);
         kring_buffer_get(kring_buf,hello,2);
            len =kring_buffer_len(kring_buf);
         printf("len ==%d\r\n",len);
          kring_buffer_put(kring_buf,hello,4);
         len =kring_buffer_len(kring_buf);
           printf("len ==%d\r\n",len);

           kring_buffer_put(kring_buf,hello,1);
         len =kring_buffer_len(kring_buf);
           printf("len ==%d\r\n",len);
     }
}
#endif