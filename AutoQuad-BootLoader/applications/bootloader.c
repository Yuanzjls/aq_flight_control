
/*
   基于rt-thread的飞控控制系统
   作者:jiezhi320-反重力航模
   QQ?:559415874   
 */ 
#include <board.h>
#include <rtthread.h>

#include "easyflash.h"
#include "app_easyflash.h"
#include "bootloader.h"

extern rt_err_t ry(void);

static void boot_entry(void* parameter)
{
    rt_thread_delay( RT_TICK_PER_SECOND/2);
	
	if (ry()==RT_EOK)
	{	
		set_env_by_name("start_iap", 0);	
		rt_kprintf("\r\n");	
		rt_kprintf("iap  successed !!\r\n");	
		rt_kprintf("\r\n");	
	    rt_kprintf("reboot to start system! \r\n");
		rt_kprintf("\r\n");	
	    //rt_thread_delay( RT_TICK_PER_SECOND/2);
		NVIC_SystemReset();
		while(1);
	   
	 //   rt_thread_delay( RT_TICK_PER_SECOND/2);		
		//Jump_To_APP();//直接跳转失败，怪了	
	}
	else
	{
		rt_kprintf("\r\n");	
		rt_kprintf("iap  failed !!\r\n");	
		rt_kprintf("\r\n");			
	    rt_kprintf("reboot…… \r\n");
	    //rt_thread_delay( RT_TICK_PER_SECOND/2);
		NVIC_SystemReset();
	}	
}


int bootloader(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("ry",
                           boot_entry, RT_NULL,
                           1024, 10, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

static u32 IAP_GetSector(u32 Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_Sector_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_Sector_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_Sector_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_Sector_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_Sector_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_Sector_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_Sector_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_Sector_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_Sector_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_Sector_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_Sector_10;
    }
    else		/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        sector = FLASH_Sector_11;
    }
    return sector;
}

void FLASH_If_Init(void)
{
    FLASH_Unlock();

    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
}


static vu32 StartSector = 0;
static vu32 EndSector = 0;
uint32_t FLASH_If_Erase(uint32_t UserStartSector,u32 bin_size)
{
    uint32_t i = 0;

    /* Get the number of the start and end sectors */
    StartSector = IAP_GetSector(UserStartSector); 
    EndSector   = IAP_GetSector(UserStartSector+bin_size);//(USER_FLASH_END_ADDRESS);

    //FLASH
    for (i = StartSector; i <= EndSector; i += 8)  
    {
        if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
        {
            // Error occurred while page erase
            return (ERROR);
        }
    }
    return (SUCCESS);
}

u8 IAP_Start(u32 bin_size)
{
    FLASH_If_Init();

    //FLASH_ITConfig(FLASH_IT_ERR|FLASH_IT_EOP, DISABLE);
    return FLASH_If_Erase(USER_FLASH_IAP_ADDRESS,bin_size);
}

void IAP_Stop(void)
{
    FLASH_Lock();
}

//static void delay(void)
//{
//     vu32 i =100;
//	 while(--i);
//}

static volatile u32 a,b;
static u8 FLASH_If_Write(__IO u32* FlashAddress,  u32* Data , u32 DataLength)
{
    uint32_t i = 0;

    for (i = 0; (i < DataLength) && (*FlashAddress <= (USER_FLASH_END_ADDRESS-4)); i++)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
           be done by word */
        if (FLASH_ProgramWord(*FlashAddress, *(uint32_t*)(Data+i)) == FLASH_COMPLETE)
        {
			// delay();
            /* Check the written value */
			a=*(uint32_t*)*FlashAddress;
			b= *(uint32_t*)(Data+i);
            if(a!=b) //if (*(uint32_t*)*FlashAddress != *(uint32_t*)(Data+i))
            {
                /* Flash content doesn't match SRAM content */
                return(2);
            }
            /* Increment FLASH destination address */
            *FlashAddress += 4;
        }
        else
        {
            /* Error occurred while writing data in Flash memory */
            return (1);
        }
    }
    return (0);
}


u32 IAP_Flash_Write(u32 addr, u8 *pbuf, u32 size)
{
    u32 ret;
    //FLASH_If_Init();
    ret=FLASH_If_Write(&addr, (u32*)pbuf, size);
    return ret;
}


void IAP_Flash_Read(u32 addr, u8 *pbuf, u32 size)
{
    u32 i;
    u32 *p = (u32*)pbuf;
    for(i=0; i < (size/sizeof(u32)); i++)
        *(p+i) = *(__IO u32*)(addr+i*4);

}


typedef  void (*fun)(void);	
fun AppStart; 


void Jump_To_APP(void)
{
	u32 JumpAddress;  
	
	if (((*(__IO uint32_t*)USER_FLASH_IAP_ADDRESS) & 0x2FFE0000 ) == 0x20000000)//检查栈顶地址是否合法
	{
		//__asm("CPSID  I");
		JumpAddress = *(volatile uint32_t*) (USER_FLASH_IAP_ADDRESS+4);
		AppStart = (fun) JumpAddress;
		
		/* Initialize user application's Stack Pointer */
		//__set_PSP(*(volatile uint32_t*) USER_FLASH_IAP_ADDRESS);
		//__set_CONTROL(0);

		__set_MSP(*(volatile uint32_t*) USER_FLASH_IAP_ADDRESS);
		AppStart();									
	}
}

