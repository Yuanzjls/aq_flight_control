/*
   基于rt-thread的飞控控制系统
   作者:jiezhi320-反重力航模
   QQ?:559415874   
 */ 
#include <board.h>
#include <rtthread.h>
#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32f4xx_eth.h"
#endif

#include "easyflash.h"
#include "app_easyflash.h"

#include <rtdevice.h>


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[512];
static struct rt_thread led_thread;
static void led_thread_entry(void* parameter);

static void rtt_user_assert_hook(const char* ex, const char* func, rt_size_t line); 
static rt_err_t exception_hook(void *context);


void rt_init_thread_entry(void* parameter)
{
    int priority = 0;
	
	/* set hardware exception hook */
	rt_hw_exception_install(exception_hook);
	/* set RT-Thread assert hook */
	rt_assert_set_hook(rtt_user_assert_hook);	
	
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
	
	print_boottimes();
}

int rt_application_init()
{
    rt_thread_t tid;
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }


    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048*2, 5, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}


//static char test[90];
static void rtt_user_assert_hook(const char* ex, const char* func, rt_size_t line) 
{  
	//sprintf(test,"(%s)  :%s, :%s\n",  func, line,rt_thread_self()->name);
	rt_kprintf("(%s) assertion failed at function:%s, line number:%d \n",  func, line);
   
    while(1);
}

static rt_err_t exception_hook(void *context)
{
	 
    while(1);

    return RT_EOK;
}


static void led_thread_entry(void* parameter)
{
    unsigned int count=0;
    unsigned int i = 0;
	
//	unsigned int test[15];

    rt_device_t pin;
    struct rt_device_pin_mode pin_mode;
    struct rt_device_pin_status pin_status;

    pin = rt_device_find("pin");
    if (pin == RT_NULL)
    {
        rt_kprintf("no pin device in the system.\n");
        return ;
    }
    for (i=0; i<3; i++)
    {
        pin_mode.pin = i;
        pin_mode.mode = PIN_MODE_OUTPUT;

        pin->control(pin, 0, &pin_mode);
    }
	
//    for (test[0]=0; test[0]<3; test[0]++)
//    {
//        pin_mode.pin = i;
//        pin_mode.mode = PIN_MODE_OUTPUT;

//        pin->control(pin, 0, &pin_mode);
//    }	
////    p4 = test;
    //rt_hw_led_init();

    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        //rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;

        for (i=0; i<3; i++)
        {
            pin_status.pin = i;
            pin_status.status = PIN_HIGH;

            pin->write(pin, 0,&pin_status, sizeof(struct rt_device_pin_status));    //rt_hw_led_on(0);
        }
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        //rt_kprintf("led off\r\n");
#endif

        for (i=0; i<3; i++)
        {
            pin_status.pin = i;
            pin_status.status = PIN_LOW;

            pin->write(pin, 0, &pin_status, sizeof(struct rt_device_pin_status)); ;    ////rt_hw_led_off(0);
        }
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}
