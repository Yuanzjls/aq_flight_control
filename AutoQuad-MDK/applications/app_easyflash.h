/*********************************************************************************************************
//                             NCLUDE FILES
*********************************************************************************************************/
#ifndef APP_EASY_FLASH_H
#define APP_EASY_FLASH_H

#include <rthw.h>	
#include <rtthread.h>
#include <stm32f4xx_conf.h>

uint32_t get_env_by_name(const char *name);

void set_env_by_name(const char *name, uint32_t value);


#endif
