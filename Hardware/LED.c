#include "LED.H"

char *Cmd_List[]=
{
	"LED0 ON\r\n",
	"LED0 OFF\r\n",
	"LED1 ON\r\n",
	"LED1 OFF\r\n"
};

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_Initstructrue;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_Initstructrue.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Initstructrue.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_5;
    GPIO_Initstructrue.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_Initstructrue);
    GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_5);
}

void LED_OFF(uint8_t LED)
{
    if(LED == 0)
    {
        GPIO_SetBits(GPIOA,GPIO_Pin_0);
    }
    else if(LED == 1)
    {
        GPIO_SetBits(GPIOA,GPIO_Pin_5);
    }
}

void LED_ON(uint8_t LED)
{
    if(LED == 0)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    }
    else if(LED == 1)
    {
        GPIO_ResetBits(GPIOA,GPIO_Pin_5);
    }
}


/********************************
函数名：LED_FUNCTION(char *str)
函数功能：根据收到的字符串控制LED的开关状态，
          并在OLED上显示对应的状态信息
输入：str - 收到的字符串
输出：无
***********************************/
void LED_FUNCTION(char *str)
{
    if(strcmp(str, "LED0 ON\r\n") == 0)
    {
        LED_ON(0);  
        OLED_ClearArea(0,0,128,16);
        OLED_ShowString(0, 0, "LED0 ON",OLED_8X16);
        
    }
    else if(strcmp(str, "LED0 OFF\r\n") == 0)
    {
        LED_OFF(0); 
        OLED_ClearArea(0,0,128,16);
        OLED_ShowString(0, 0, "LED0 OFF",OLED_8X16);
    }
    else if(strcmp(str, "LED1 ON\r\n") == 0)
    {
        LED_ON(1); 
        OLED_ClearArea(0,16,128,16);
        OLED_ShowString(0, 16, "LED1 ON",OLED_8X16);
    }
    else if(strcmp(str, "LED1 OFF\r\n") == 0)
    {
        LED_OFF(1); 
        OLED_ClearArea(0,16,128,16);
        OLED_ShowString(0, 16, "LED1 OFF",OLED_8X16);
    }
    printf("进行了%s\r\n",str);
}

/********************************
函数名：LED_Detect(char *str)
函数功能：检测收到的字符串是否与指定字符串匹配，
          如果匹配则打开对应的LED并在OLED上显示
输入：length - 需要读取的数据长度
输出：无
***********************************/
void LED_Detect(char *str)
{
    uint16_t Length=0;
    uint16_t count=0;
    
    Length = My_Strlen(Queue_Buffer,Queue_Buffer + Queue_read_index, Buffer_Size_Max);
    if(Length >= strlen(str))
    {
        for(uint16_t i=0;i<Length;i++)
        {
            if(Queue_Buffer[(Queue_read_index+i)%(Buffer_Size_Max)] == *(str+count))
            {
                count++;
                if(count>=strlen(str))
                {
                    Index_offset = i;
                    break;
                }
            }
        }
    } 
    
    if(count==strlen(str))
    {
        LED_FUNCTION(str);
		printf("进行了%s\r\n",str);
    }
}


void LED_Detect_All(void)
{
    uint16_t Length=0;
    uint16_t count[]={0};
    
    Length = My_Strlen(Queue_Buffer,Queue_Buffer + Queue_read_index, Buffer_Size_Max);
    for(uint16_t i=0;i<Length;i++)
    {
        for(uint8_t j=0;j<4;j++)
        {
            if(Queue_Buffer[(Queue_read_index+i)%(Buffer_Size_Max)] == *(Cmd_List[j]+count[j]))
            {
                count[j]++;
                if(count[j]>=strlen(Cmd_List[j]))
                {
                    count[j]=0; 
                    Index_offset = i+1;
                    LED_FUNCTION(Cmd_List[j]);
                    goto exit_loop; 
                }
            }
            else{count[j]=0;}
        }

    }
    exit_loop:
    return;
}
