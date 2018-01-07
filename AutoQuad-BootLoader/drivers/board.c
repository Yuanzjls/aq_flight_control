/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"
#include "usart.h"
#include "gpio.h"
#include "easyflash.h"

/**
 * @addtogroup STM32
 */

/*@{*/
extern void rccConfiguration(void);

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This fucntion returns milliseconds since system passedºÁÃë
 */
rt_uint32_t rt_hw_tick_get_millisecond(void)
{
    rt_tick_t tick;
    rt_uint32_t value;

#define TICK_MS (1000/RT_TICK_PER_SECOND)

    tick = rt_tick_get();
    value = tick * TICK_MS + (SysTick->LOAD - SysTick->VAL) * TICK_MS / SysTick->LOAD;

    return value;
}

/**
 * This fucntion returns microseconds since system passedÎ¢Ãë
 */
rt_uint32_t rt_hw_tick_get_microsecond(void)
{
    rt_tick_t tick;
    rt_uint32_t value;

#define TICK_US	(1000000/RT_TICK_PER_SECOND)

    tick = rt_tick_get();
    value = tick * TICK_US + (SysTick->LOAD - SysTick->VAL) * TICK_US / SysTick->LOAD;

    return value;
}


/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init()
{
    /* NVIC Configuration */
    NVIC_Configuration();

    /* Configure the SysTick */
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

	easyflash_init();
	
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#else
    //stm32_hw_usart_init();
    stm32_hw_pin_init();
#endif		

    
#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void reboot(void)
{
    NVIC_SystemReset();
}

FINSH_FUNCTION_EXPORT(reboot, reset the system);
#endif
