/*
   基于rt-thread的飞控控制系统
   作者:jiezhi320-反重力航模
   QQ?:559415874   
 */ 
#include <rtthread.h>
#include <ymodem.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>

#include "bootloader.h"


struct custom_ctx
{
    struct rym_ctx parent;
    int fd;
    int flen;
    char fpath[256];
	int size_write;
};

static enum rym_code _rym_bg(
        struct rym_ctx *ctx,
        rt_uint8_t *buf,
        rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx*)ctx;
    cctx->fpath[0] = '/';
    /* the buf should be the file name */
    strcpy(&(cctx->fpath[1]), (const char*)buf);
   // cctx->fd = open(cctx->fpath, O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (cctx->fd < 0)
    {
        rt_err_t err = rt_get_errno();
        rt_kprintf("error creating file: %d\n", err);
        rt_kprintf("abort transmission\n");
        return RYM_CODE_CAN;
    }

    cctx->flen = atoi((const char*)buf+strlen((const char*)buf)+1);
	//得到文件大小，根据大小擦除flash区块

	if(IAP_Start(cctx->flen) != SUCCESS)
	{
    //  rt_kprintf("flash erase failed \n");
	//	rt_kprintf("reboot \r\n");
		NVIC_SystemReset();
        while(1);
	}
	rt_thread_delay(5);//放慢 免得一些数传反应 不及时导致升级失败
	//rt_kprintf("app file size is %d \r\n",cctx->flen);
	cctx->size_write = 0;
    if (cctx->flen == 0)
        cctx->flen = -1;
    return RYM_CODE_ACK;
}

static enum rym_code _rym_tof(
        struct rym_ctx *ctx,
        rt_uint8_t *buf,
        rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx*)ctx;
    //RT_ASSERT(cctx->fd >= 0);
    if (cctx->flen == -1)
    {
		rt_kprintf("flen is -1 \r\n");
		if (IAP_Flash_Write(USER_FLASH_IAP_ADDRESS + cctx->size_write, buf, len/4)!=0)
		{	
		//	rt_kprintf("flash write failed \r\n");
		    NVIC_SystemReset();
            while(1);
		}	
		cctx->size_write +=len;//记录写入的字节数
        //write(cctx->fd, buf, len);
	//	rt_kprintf("flash have write %d size\r\n",cctx->size_write);
    }
    else
    {
        int wlen = len > cctx->flen ? cctx->flen : len;
        //write(cctx->fd, buf, wlen);
		if (IAP_Flash_Write(USER_FLASH_IAP_ADDRESS + cctx->size_write, buf, wlen/4)!=0)
		{	
		//	rt_kprintf("flash write failed \r\n");
		    NVIC_SystemReset();
            while(1);
		}	
		
		cctx->size_write +=wlen;//记录写入的字节数
		//rt_kprintf("flash have write %d size\r\n",cctx->size_write);
        cctx->flen -= wlen;
		rt_thread_delay(5);//放慢 免得一些数传反应 不及时导致升级失败
    }
    return RYM_CODE_ACK;
}

static enum rym_code _rym_end(
        struct rym_ctx *ctx,
        rt_uint8_t *buf,
        rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx*)ctx;

   // RT_ASSERT(cctx->fd >= 0);
   // close(cctx->fd);
	IAP_Stop();
    cctx->fd = -1;

    return RYM_CODE_ACK;
}

rt_err_t rym_write_to_file(rt_device_t idev)
{
    rt_err_t res;
    struct custom_ctx *ctx = rt_malloc(sizeof(*ctx));

    RT_ASSERT(idev);

    rt_kprintf("entering RYM mode\n");

    res = rym_recv_on_device(&ctx->parent, idev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                             _rym_bg, _rym_tof, _rym_end, 1000);

    /* there is no Ymodem traffic on the line so print out info. */
    rt_kprintf("leaving RYM mode with code %d\n", res);
   // rt_kprintf("file %s has been created.\n", ctx->fpath);

    rt_free(ctx);

    return res;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
rt_err_t ry(void)
{
    rt_err_t res;
	

    rt_device_t dev = rt_device_find(RT_CONSOLE_DEVICE_NAME);	
    if (!dev)
    {
        rt_kprintf("could not find device:%s\n", RT_CONSOLE_DEVICE_NAME);
        return -RT_ERROR;
    }

    res = rym_write_to_file(dev);

    return res;
}
FINSH_FUNCTION_EXPORT(ry, receive files by ymodem protocol);
#endif
