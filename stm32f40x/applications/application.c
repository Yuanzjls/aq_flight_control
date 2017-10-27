/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup.
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

#include <rtdevice.h>
#include "aq_init.h"

#include "alt_ukf.h"
#include "analog.h"
#include "aq_mavlink.h"
#include "aq_timer.h"
#include "can.h"
#include "comm.h"
#include "config.h"
#include "control.h"
#include "filer.h"
#include "gimbal.h"
#include "gps.h"
#include "hilSim.h"
#include "imu.h"
#include "logger.h"
#include "motors.h"
#include "nav_ukf.h"
#include "nav.h"
#include "radio.h"
#include "rtc.h"
#include "run.h"
#include "sdio.h"
#include "aq_serial.h"
#include "signaling.h"
#include "supervisor.h"
#include "util.h"
#ifdef HAS_AQ_TELEMETRY
#include "command.h"
#include "telemetry.h"
#endif
#ifdef CAN_CALIB
#include "canCalib.h"
#endif
#ifdef HAS_TELEM_SMARTPORT
#include "telem_sPort.h"
#endif
#include <cpuusage.h>

volatile unsigned long counter;
volatile unsigned long minCycles = 0xFFFFFFFF;

digitalPin *tp;

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
static struct rt_thread led_thread;

//volatile int *p4;

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
        rt_kprintf("led on, count : %d\r\n",count);
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
        rt_kprintf("led off\r\n");
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

void rt_init_thread_entry(void* parameter)
{
    int priority = 0;
	
	
    cpu_usage_init();
	
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
//    rt_components_init();
#endif
#ifdef EEPROM_CS_PORT
    // TODO: clean this up
    digitalPin *eepromCS = digitalInit(EEPROM_CS_PORT, EEPROM_CS_PIN, 1);
#endif
#ifdef DAC_TP_PORT
    tp = digitalInit(DAC_TP_PORT, DAC_TP_PIN, 0);
#endif
    rtcInit();	    // have to do this first as it requires our microsecond timer to calibrate
    timerInit();    // now setup the microsecond timer before everything else
    commNoticesInit();  // set up notice queue
    sdioLowLevelInit();
    filerInit();
    configInit();
    signalingInit();
    supervisorInit();
    commInit();
#ifdef USE_MAVLINK
    mavlinkInit();
#endif
#ifdef HAS_AQ_TELEMETRY
    telemetryInit();
#endif
#ifdef HAS_TELEM_SMARTPORT
    if (((uint32_t)p[TELEMETRY_RX_CFG] & 0xF) == 1)
        sPortInit();
#endif
    imuInit();
    analogInit();
    navUkfInit();
    altUkfInit();
    radioInit();
    gpsInit();
    navInit();
#ifdef HAS_AQ_TELEMETRY
    commandInit();
#endif
    loggerInit();
#ifdef CAN_CALIB
    canCalibInit();
#else
    //canInit();
#endif
    hilSimInit();
    motorsInit();
    controlInit();
    /*gimbalInit();*/
    runInit();

    info();

    supervisorInitComplete();

    // allow tasks to startup
    yield(10);

    AQ_NOTICE("Initialization complete, READY.\n");

    // startup complete, reduce comm task priority
    //if (commTask)
    {
        priority = COMM_PRIORITY;
        rt_thread_control(&commTask,RT_THREAD_CTRL_CHANGE_PRIORITY,&priority);//CoSetPriority(commData.commTask, COMM_PRIORITY);
    }

#ifdef HAS_AQ_TELEMETRY
    // start telemetry
    telemetryEnable();
#endif

    // reset idle loop calibration now that everything is running
    minCycles = 999999999;
}

int rt_application_init()
{
    rt_thread_t tid;
//    rt_err_t result;

//    /* init led thread */
//    result = rt_thread_init(&led_thread,
//                            "led",
//                            led_thread_entry,
//                            RT_NULL,
//                            (rt_uint8_t*)&led_stack[0],
//                            sizeof(led_stack),
//                            20,
//                            5);
//    if (result == RT_EOK)
//    {
//        rt_thread_startup(&led_thread);
//    }


    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048*2, 12, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

//void HardFault_Handler(void) {
//    // to avoid the unpredictable flight in case of problems
//    motorsOff();

//    // Go to infinite loop when Hard Fault exception occurs
//    while (1)
//        ;
//}

