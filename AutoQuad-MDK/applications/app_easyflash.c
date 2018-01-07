/*
   基于rt-thread的飞控控制系统
   作者:jiezhi320-反重力航模
   QQ?:559415874   
 */ 
#include "app_easyflash.h"
#include <stdlib.h>
#include <stdio.h>
#include "easyflash.h"

/**
 * Env demo.
 */

uint32_t get_env_by_name(const char *name)
{
    uint32_t value = 0;
    char *c_name = {0};

	c_name = ef_get_env(name);
    RT_ASSERT(c_name);
    value = atol(c_name);
   
    return value;
}	

void set_env_by_name(const char *name, uint32_t value)
{
    char c_name[20] = {0};
	
	/* interger to string */
    sprintf(c_name,"%ld", value);
    /* set and store the boot count number to Env */
    ef_set_env(name, c_name);
    ef_save_env();
}
