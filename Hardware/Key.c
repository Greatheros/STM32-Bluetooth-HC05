#include "Key.h"

//适应共阴共阳两种方式，默认共阴,若使用共阳极连接，请改为#define Common_Yin 0
#define Common_Yin 1

//按键状态数组
KEY_STATE Key_State[KEY_MAX];

//按键列表，创建的按键会被添加到这里
KEY_TypeDef Key_List[KEY_MAX]={0};

//按键结构体数组
static KEY_T Button[KEY_MAX];


void TIM2_ITInit(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_InitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period=72-1;
    TIM_InitStructure.TIM_Prescaler=1000-1;
    TIM_InitStructure.TIM_RepetitionCounter=0;
    TIM_TimeBaseInit(TIM2,&TIM_InitStructure);
    TIM_Cmd(TIM2,ENABLE);

    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
    NVIC_Init(&NVIC_InitStructure);
}

/**************************
 *函数功能：创建一个按键，添加到Key_List数组中
 *输入：id，给按键一个编号，编号必须唯一，且不能大于KEY_MAX
 *      GPIO，按键所在GPIO的端口
 *      GPIO_Pin，按键所在的pin口
 *输出：NULL
***************************/
void Create_Key(uint16_t id,GPIO_TypeDef *GPIO,uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    if(id<KEY_MAX)
    {
        Key_List[id].gpio=GPIO;
        Key_List[id].gpio_pin=GPIO_Pin;
        
        // 初始化按键结构体
        Button[id].State = KEY_IDLE;
        Button[id].repeat = keyFALSE;
        Button[id].Repeat_time=200;
        Button[id].Filter_Cnt= 0;
        Button[id].Callback=NULL;

        if(Key_List[id].gpio == GPIOA) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        }
        else if(Key_List[id].gpio == GPIOB) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        }
        else if(Key_List[id].gpio == GPIOC) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        }
        else if(Key_List[id].gpio == GPIOD) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        }
        else if(Key_List[id].gpio == GPIOE) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
        }
        if(Common_Yin==1){GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;}
        else{GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;}
        GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin=Key_List[id].gpio_pin;
        GPIO_Init(Key_List[id].gpio,&GPIO_InitStructure);
    }
}


/**************************
 *函数功能：判断按键是否按下（默认共阴极连接，若为共阳极，请修改宏#define Common_Yin使之不为1）
 *输入：id,列表按键的id
 *输出：KEY_Press表示按键按下,KEY_NoPress表示按键未按下
***************************/
uint16_t Key_Active(uint16_t id)
{
   
    #if Common_Yin
        uint8_t Res=1;
        Res = GPIO_ReadInputDataBit(Key_List[id].gpio,Key_List[id].gpio_pin);
        if(Res==0)
        {
            return KEY_Press;
        }
        else{return KEY_NoPress;}
    #else
        uint8_t Res=0;
        Res = GPIO_ReadInputDataBit(Key_List[id].gpio,Key_List[id].gpio_pin);
        if(Res==1)
        {
            return KEY_Press;
        }
        else{return KEY_NoPress;}
    #endif
}

/**************************
 *函数功能：设置按键是否重复触发（长按状态）
 *输入：id,列表按键的id
 *     repeat，keyTRUE重复，keyFALSE不重复
 *输出：NULL
***************************/
void Button_Setrep(uint16_t id,uint8_t repeat)
{
    Button[id].repeat=repeat;
}

/**************************
 *函数功能：设置按键每段重复触发的间隔
 *输入：id,列表按键的id
 *     time，期望的重复触发的间隔，单位ms
 *输出：NULL
***************************/
void Button_RepTime(uint16_t id,uint8_t time)
{
    Button[id].Repeat_time=time;
}

/**************************
 *函数功能：创建节点
 *输入：handler，回调函数地址
 *     
 *输出：新创建的节点的地址
***************************/
static Node* create_callback_node(Key_Vhandler handler) {
    Node* node = malloc(sizeof(Node));      //申请空间，返回地址
    if (node) {
        node->KeyCallBack1=handler;
        node->next=NULL;
    }
    return node;
}

/**************************
 *函数功能：按键回调函数注册
 *输入：id,列表按键的id
 *     handler，回调函数地址
 *输出：NULL
***************************/
void Key_Register(uint16_t id,Key_Vhandler handler)
{
    if(id<=KEY_MAX)
    {
        Node* Newnode = create_callback_node(handler);
        Newnode->next=Button[id].Callback;
        Button[id].Callback=Newnode;
        //Button[id].KeyCallBack1=handler;
    }
}

/**************************
 *函数功能：触发回调函数
 *输入：id,列表按键的id
 *     state，按键状态
 *输出：NULL
***************************/
void Tricalback(uint16_t id,KEY_STATE state)
{
    Node* Now_Code = Button[id].Callback;
    while(Now_Code!=NULL)
    {
        Now_Code->KeyCallBack1(state);   //执行回调
        Now_Code=Now_Code->next;        //更新地址
    }
}

/**************************
 *函数功能：核心函数，必须周期性调用，用于扫描按键判断状态
 *输入：button,结构体地址
 *     id,列表按键的id
 *输出：NULL
***************************/
void KEY_Scan(KEY_T *button,uint16_t id)
{

    static uint16_t LongTime[KEY_MAX]={0};
    static uint16_t DoubleTime[KEY_MAX]={0};
    static uint8_t ClickStage[KEY_MAX] = {0}; // 0: 无点击 1: 双击后  2：长按后

    switch (button->State)
    {
        /*空闲状态，按键没有按下*/
        case KEY_IDLE:
            button->Filter_Cnt=0;
            LongTime[id]=0;
            DoubleTime[id]=0;
            if(Key_Active(id)==KEY_Press )
            {
                button->State=KEY_PRESS;
            }
            
        break;

        /*按键按下，进行消抖*/
        case KEY_PRESS:
            /*消抖*/
            if(button->Filter_Cnt < KEY_Filter)
            {
                button->Filter_Cnt=KEY_Filter;
            }
            else if(button->Filter_Cnt < 2*KEY_Filter)
            {
                button->Filter_Cnt++;
            }
            
            else{
                if(ClickStage[id]!=1)              
                {
                    /*如果在长按检测时间内按键松开了*/
                    if(LongTime[id] < KEY_Longtime)
                    {
                        LongTime[id]++;
                        if(Key_Active(id)!=KEY_Press)          
                        {
                            /*如果松开的时间在双击检测时间内*/
                            if(ClickStage[id]==0)
                            {
                                button->State=KEY_RELEASE;
                            }
                            else{ button->State=KEY_IDLE;ClickStage[id]=0;}
                        }
                    }
                    else{
                        button->State=KEY_LONG;                 //长按
                        ClickStage[id]=2;
                    }
                }
                else if(ClickStage[id]==1)
                {
                    if(Key_Active(id)!=KEY_Press)
                    {
                        button->State=KEY_DOUBLE;    //双击
                        ClickStage[id]=0;
                    }
                }

            }
        break;

        //按键释放状态
        case KEY_RELEASE:

            if(DoubleTime[id]<KEY_Doubletime)
            {
                if(Key_Active(id)==KEY_Press)
                {
                    button->State=KEY_PRESS;    //再次进入按下状态
                    button->Filter_Cnt=KEY_Filter;
                    ClickStage[id]=1;          //状态为双击后
                }
                DoubleTime[id]++;
            }
            else{
                button->State=KEY_SINGLE;        //单击
            }
            
        break;
            
    }

    switch (button->State)
    {
        //调用单次的回调函数
        case KEY_SINGLE: 
            if(button->Callback!=NULL)
            {
                Tricalback(id,KEY_SINGLE);
            }
            button->State=KEY_IDLE;
        break;

        //调用双击的回调函数
        case KEY_DOUBLE: 
            if(button->Callback!=NULL)
            {
                Tricalback(id,KEY_DOUBLE);
            }
            button->State=KEY_IDLE;
            
        break;  

        //调用长按的回调函数
        case KEY_LONG: 
            if(button->Callback!=NULL)
            {
                Tricalback(id,KEY_LONG);
            }
            if(Button[id].repeat!=keyTRUE)
            {
                button->State=KEY_IDLE;
            }
            else{button->State=KEY_REPEAT;
                 LongTime[id]=button->Repeat_time;
            }
            
        break; 

        //调用重复的回调函数
        case KEY_REPEAT:
            
            if(Key_Active(id)==KEY_Press)
            {
                if( LongTime[id]%button->Repeat_time==0)
                {
                    if(button->Callback!=NULL)
                    {
                        Tricalback(id,KEY_REPEAT);
                    }
                }
                LongTime[id]++;
            }
            else{button->State=KEY_IDLE;ClickStage[id]=0;}
        break;    
    }
}

/**************************
 *函数功能：放入1ms定时器中断中，判断按键是否创建，若创建则进行扫描
 *输入：NULL
 *输出：NULL
***************************/
void Key_Tick(void)
{
    for(int id=0;id<KEY_MAX;id++)
    {
        if(Key_List[id].gpio!=NULL && Key_List[id].gpio_pin!=NULL)
        {
            KEY_Scan(&Button[id],id);
        }
    }
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
        Key_Tick();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
