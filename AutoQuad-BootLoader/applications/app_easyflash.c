/*
   基于rt-thread的飞控控制系统
   作者:jiezhi320-反重力航模
   QQ?:559415874   
 */ 
#include "app_easyflash.h"
#include <stdlib.h>
#include <stdio.h>
#include "easyflash.h"
#include "finsh.h"
#include "shell.h"
#include "bootloader.h"

/**
 * Env demo.
 */
void print_boottimes(void)
{
    uint32_t i_boot_times = 0, start_iap = 0, build_num = 0;
	
	build_num = get_env_by_name("buildnum"); 
	
	//变量内容有变化后 修改BUILDNUMBER 数值 让数据进行一次刷新
    if (build_num!=BUILDNUMBER)
    {
     //   result = ef_port_erase(EF_START_ADDR, ENV_AREA_SIZE);
        ef_env_set_default();
		
		set_env_by_name("buildnum", BUILDNUMBER);
		rt_kprintf("set env to default\r\n");
    }	
	
    /* get the boot count number from Env */
    i_boot_times = get_env_by_name("boot_times");
    /* boot count +1 */
    i_boot_times ++;
	
    rt_kprintf("The AQ system now boot %d times\n", i_boot_times);
	rt_kprintf("jiezhi320QQ Group: 559415874 \r\n", i_boot_times);
	
	set_env_by_name("boot_times", i_boot_times);
    
	start_iap = get_env_by_name("start_iap");
	
	if (0!=start_iap)
	{
	    rt_kprintf("start AQ BootLoader system \r\n");
        bootloader();		
	}	
	else
	{	
	     rt_kprintf("start AQ system \r\n");
		 Jump_To_APP();	
	}	
}

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
