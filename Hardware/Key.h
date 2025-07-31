#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header
#include <stddef.h>
#include <stdlib.h>

#define KEY_MAX  3           //最大按键数量

#define KEY_Press    1        //按键按下
#define KEY_NoPress  0        //按键没有按下
#define KEY_Filter   20       //消抖时长
#define KEY_Longtime 1500     //按键长按时间
#define KEY_Doubletime 300    //按键双击间隔

#define keyTRUE  1
#define keyFALSE 0


//按键列表定义
typedef struct
{
    GPIO_TypeDef *gpio;
    uint16_t    gpio_pin;
}KEY_TypeDef;

//按键状态
typedef enum
{
    KEY_IDLE,        //空闲
    KEY_PRESS,       //按下
    KEY_RELEASE,     //释放
    
    /*下面4个状态可外部使用*/
    KEY_SINGLE,      //单击
    KEY_DOUBLE,      //双击
    KEY_LONG,        //长按
    KEY_REPEAT,      //重复
}KEY_STATE;


typedef void (*Key_Vhandler) (KEY_STATE State);           //自定义与之结构相似的函数，然后用Key_Register注册
//链表结构
typedef struct Node
{
    Key_Vhandler KeyCallBack1; 
    struct Node *next;
}Node;

//每个按键对应一个按键结构体

typedef struct 
{
    KEY_STATE State;            //状态
    uint8_t   repeat;           //是否启用重复触发,keyTRUE重复，keyFALSE不重复
    uint8_t   Repeat_time;      //每段重复触发的间隔时长
    uint16_t  Filter_Cnt;       //消抖计数
    Node      *Callback;        //回调结构体指针
}KEY_T;


//外部可用函数
void TIM2_ITInit(void);                                                     //启动定时器2
void Create_Key(uint16_t id,GPIO_TypeDef *GPIO,uint16_t GPIO_Pin);          //创建一个按键
void Key_Register(uint16_t id,Key_Vhandler handler);                        //注册按键回调函数
void Button_Setrep(uint16_t id,uint8_t repeat);                             //设置按键是否重复触发
void Button_RepTime(uint16_t id,uint8_t time);                              //设置按键重复触发的间隔时长,默认200
void Key_Tick(void);                                                        //将他放到一个1ms定时器中

#endif
