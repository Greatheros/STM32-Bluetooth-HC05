#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "serial.h"
#include "serial.h"

void LED_Init(void);
void LED_OFF(uint8_t LED);
void LED_ON(uint8_t LED);
void LED_Detect(char *str);
void LED_Detect_All(void);

#endif
