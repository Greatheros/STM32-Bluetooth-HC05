#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "serial.h"


int main(void)
{
	/*OLED初始化*/
	OLED_Init();
	LED_Init();
	Serial_Init();
	OLED_ShowString(0, 0, "LED0 OFF",OLED_8X16);
	OLED_ShowString(0, 16, "LED1 OFF",OLED_8X16);
	OLED_Update();
	
	while (1)
	{
		LED_Detect_All();
		Queue_read();
		
		/*********************调试可用*********************/
		//OLED_ShowNum(0,48,Queue_write_index,3,OLED_6X8);
		//OLED_ShowNum(0,56,Queue_read_index,3,OLED_6X8);
		/*********************调试可用*********************/
		
		OLED_Update();
	}
}
