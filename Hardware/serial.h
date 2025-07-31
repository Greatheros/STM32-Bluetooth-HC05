#ifndef  __SERIAL_H
#define  __SERIAL_H

#include <stdio.h>
#include <string.h>
#include "stdarg.h"

#define Buffer_Size_Max 128  //缓冲区大小

extern char USART1_RX_BUF[Buffer_Size_Max];
extern char Queue_Buffer[Buffer_Size_Max];   
extern uint16_t USART1_Write_index;
extern uint16_t Queue_write_index;
extern uint16_t Queue_read_index;
extern uint16_t Index_offset;


void Serial_Init(void);
uint16_t My_Strlen(char *Parent,const char *Substr,uint16_t Length);
void Queue_read(void);

#endif
