#include "stm32f10x.h"                  // Device header
#include "serial.h"

char USART1_RX_BUF[Buffer_Size_Max];  //接收缓冲区
char Queue_Buffer[Buffer_Size_Max];    //实际读取数据队列

uint16_t USART1_Write_index=0;  //接收缓冲区写入位置
uint16_t Readindex=0;           //接收缓冲区读索引
uint16_t Readindex_bck=0;        //接收缓冲区辅助读索引

uint16_t Queue_write_index = 0;  //队列写入位置
uint16_t Queue_read_index = 0;   //队列读取位置

uint16_t Index_offset = 0;      // 队列偏移量，用于处理环形缓冲区


struct __FILE 
{ 
	int handle; 
 
}; 
 
FILE __stdout;       

void _sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);
    USART1->DR = (uint8_t) ch;      
	return ch;
}

void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructrue;
	GPIO_InitStructrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructrue.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStructrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructrue);
	
	GPIO_InitStructrue.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructrue.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructrue);
	
	USART_InitTypeDef USART_InitStructrue;
	USART_InitStructrue.USART_BaudRate=9600;        
	USART_InitStructrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None;          //硬件流控制
	USART_InitStructrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; ;                  
	USART_InitStructrue.USART_Parity=USART_Parity_No;               //是否奇偶检验  
	USART_InitStructrue.USART_StopBits=USART_StopBits_1;           //停止位字节长度
	USART_InitStructrue.USART_WordLength=USART_WordLength_8b;       //发送|接受数据长度
	USART_Init(USART1,&USART_InitStructrue);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);       //中断条件配置,接收数据寄存器不空中断
	
	//*******NVIC优先级设置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_Initstructrue;
	NVIC_Initstructrue.NVIC_IRQChannel=USART1_IRQn;
	NVIC_Initstructrue.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Initstructrue.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_Initstructrue.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_Initstructrue);

	USART_Cmd(USART1,ENABLE);
}

/********************************
函数名：uint16_t My_Strlen(char *Parent,const char *Substr,uint16_t Length)
函数功能：环形计算子字符串在父字符串中的长度，即支持子字符串被分割成父字符串两端的情况
输入：length - 需要读取的数据长度
       Parent - 父字符串的起始地址
       Substr - 子字符串的起始地址
       Length - 父字符串的长度
输出：子字符串的长度
***********************************/
uint16_t My_Strlen(char *Parent,const char *Substr,uint16_t Length)
{
    uint16_t count = 0;
	while (*Substr != '\0')
	{
		Substr++;
        if(Substr >= Parent + Length ) // 防止越界
        {
            Substr=Parent;
        }
		count++;
	}
	return  count;
}

/********************************
函数名：Queue_commnd()
函数功能：进行写入队列（Queue_Buffer）的操作，并更新队列写索引（Queue_write_index）
输入：无
输出：无
***********************************/
void Queue_commnd(void)
{
	uint16_t Length = My_Strlen(USART1_RX_BUF, (const char *)&USART1_RX_BUF[Readindex], Buffer_Size_Max);  //计算需要写入队列的长度
	if(Readindex > USART1_Write_index)
	{
		uint16_t error = Buffer_Size_Max - Readindex;
		if(Queue_write_index + Length >= Buffer_Size_Max)
		{
			uint16_t remaining_space = Buffer_Size_Max - Queue_write_index;
			
			if(error >= remaining_space)
			{
				strncpy(Queue_Buffer + Queue_write_index, (const char *)&USART1_RX_BUF[Readindex], remaining_space);
				strncpy(Queue_Buffer, (const char *)&USART1_RX_BUF[Readindex + remaining_space], error - remaining_space);
				strcpy(Queue_Buffer+ error - remaining_space, (const char *)USART1_RX_BUF);
			}
			else{
				strncpy(Queue_Buffer + Queue_write_index, (const char *)&USART1_RX_BUF[Readindex], error);
				remaining_space-= error;
				strncpy(Queue_Buffer + Queue_write_index + remaining_space, (const char *)USART1_RX_BUF, remaining_space);
				strcpy(Queue_Buffer, (const char *)&USART1_RX_BUF[remaining_space]);
			}
		}
		else
		{
			strncpy(Queue_Buffer + Queue_write_index, (const char *)&USART1_RX_BUF[Readindex], error);
			strcpy(&Queue_Buffer[error + Queue_write_index], (const char *)USART1_RX_BUF);
		}
	}
	else
	{
		if(Queue_write_index + Length >= Buffer_Size_Max)
		{
			uint16_t remaining_space = Buffer_Size_Max - Queue_write_index;
			strncpy(Queue_Buffer + Queue_write_index, (const char *)&USART1_RX_BUF[Readindex], remaining_space);
			strncpy(Queue_Buffer, (const char *)&USART1_RX_BUF[Readindex + remaining_space], Length - remaining_space);
		}
		else
		{
			strcpy(Queue_Buffer + Queue_write_index, (const char *)&USART1_RX_BUF[Readindex]);
		}
	}
	Queue_write_index = (Queue_write_index + Length) % Buffer_Size_Max;  //更新队列写入位置
}

/********************************
函数名：Queue_read(uint16_t length)
函数功能：更新队列的读索引（Queue_read_index）
输入：无
输出：无
***********************************/
void Queue_read(void)
{
	Queue_read_index = (Queue_read_index + Index_offset )%(Buffer_Size_Max);  //更新队列读取位置
	Index_offset = 0;  //重置偏移量
}


void USART1_IRQHandler(void)
{
	static uint8_t last_data=0;  //上次接收的数据
	static uint8_t data=0; 
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		last_data = data;
		data=USART_ReceiveData(USART1);

		USART1_RX_BUF[USART1_Write_index]=data;  //将接收到的数据存入缓冲区
		USART1_Write_index=(USART1_Write_index+1)%Buffer_Size_Max;    //如果超出缓冲区大小，则从头开始写入

		if(last_data=='\r')
		{
			if(data=='\n'){  //如果接收到回车换行符，则清空缓冲区
				USART1_RX_BUF[USART1_Write_index]='\0';
				//USART1_Write_index=(USART1_Write_index+1)%Buffer_Size_Max;
				Readindex = Readindex_bck;  //将读取位置重置为上次读取位置
				Readindex_bck = USART1_Write_index;
				Queue_commnd();
			}
		}


		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
	
}
