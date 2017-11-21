/*************************************************************************************
* Test-program for Olimex �0�3��STM32-H103, header board for �0�3��STM32F103RBT6.
* After program start green LED (LED_E) will blink.
*
* Program has to be compiled with optimizer setting "-O0".
* Otherwise delay via while-loop will not work correctly.
*************************************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stdio.h"
#include <string.h>
#include "fsm.h"
#include "dev_obj.h"

#define RX_BUF_SIZE 80
volatile char RX_FLAG_END_LINE = 0;
volatile char RXi;
volatile char RXc;
volatile char RX_BUF[RX_BUF_SIZE] = {'\0'};
volatile char buffer[80] = {'\0'};

void clear_RXBuffer(void) {
	for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
		RX_BUF[RXi] = '\0';
	RXi = 0;
}

void usart_dma_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA */
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&buffer[0];
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_BufferSize = sizeof(buffer);
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStruct);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
		- BaudRate = 115200 baud
		- Word Length = 8 Bits
		- One Stop Bit
		- No parity
		- Hardware flow control disabled (RTS and CTS signals)
		- Receive and transmit enabled
		- USART Clock disabled
		- USART CPOL: Clock is active low
		- USART CPHA: Data is captured on the middle
		- USART LastBit: The clock pulse of the last data bit is not output to
			the SCLK pin
	 */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	//DMA_Cmd(DMA1_Channel4, ENABLE);

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);


	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
	USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USART1_IRQHandler(void)
{
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
    		RXc = USART_ReceiveData(USART1);
    		RX_BUF[RXi] = RXc;
    		RXi++;

    		if (RXc != 13) {
    			if (RXi > RX_BUF_SIZE-1) {
    				clear_RXBuffer();
    			}
    		}
    		else {
    			RX_FLAG_END_LINE = 1;
    		}
			//Echo
    		USART_SendData(USART1, RXc);
	}
}

void USARTSendDMA(char *pucBuffer)
{
	strcpy((char*)buffer, pucBuffer);
	/* Restart DMA Channel*/
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = strlen(pucBuffer);
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

void DMA1_Channel4_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC4);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	
	/* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
}


volatile int TimeResult;
volatile int TimeSec;
volatile uint8_t TimeState = 0;

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		TimeSec++;
	}
}

void app_set(void) {
	#define IAP_FLASH_SIZE  0x3000
	#define ApplicationAddress  0x8003000
	NVIC_SetVectorTable(ApplicationAddress,IAP_FLASH_SIZE);
}

void gpio_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* GPIOC Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);

	/* Configure PC12 to mode: slow rise-time, pushpull output */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; // GPIO No. 12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // slow rise time
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // push-pull output
	GPIO_Init(GPIOA, &GPIO_InitStructure); // GPIOC init

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4; // GPIO No. 12
	GPIO_Init(GPIOB, &GPIO_InitStructure); // GPIOC init

	GPIO_WriteBit(GPIOA,GPIO_Pin_15,(BitAction)!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15));
	GPIO_WriteBit(GPIOA,GPIO_Pin_15,(BitAction)!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15));

//	GPIO_WriteBit(GPIOB,GPIO_Pin_3,(BitAction)!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3));
//	GPIO_WriteBit(GPIOB,GPIO_Pin_3,(BitAction)!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3));

//	GPIO_WriteBit(GPIOB,GPIO_Pin_4,(BitAction)!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));
//	GPIO_WriteBit(GPIOB,GPIO_Pin_4,(BitAction)!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));
}

void time_init(void) {
	// TIMER4
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 

  	TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIMER_InitStructure.TIM_Prescaler = 2400; 
    TIMER_InitStructure.TIM_Period = 500; 
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); 
    TIM_Cmd(TIM4, ENABLE);

    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

simple_fsm(LedTask3,)
fsm_initialiser(LedTask3,
	l_WaitX(0,5);  
	USARTSendDMA("xxxxwait_5-----\r\n");
	l_WaitX(1,10);  
	USARTSendDMA("wait_10-----\r\n");
	l_WaitX(2,15);  
	USARTSendDMA("wait_15----\r\n");
	l_WaitX(3,20);  
	USARTSendDMA("wait_25---\r\n");
	l_WaitX(5,25);  
	USARTSendDMA("wait_30--\r\n");
	l_WaitX(6,30);  
	USARTSendDMA("wait_35---\r\n");
	l_WaitX(7,35);  
	USARTSendDMA("xxwait_40---\r\n");
)

simple_fsm(LedTask,)
fsm_initialiser(LedTask,
	while(1) {
		l_WaitX(0,5);  
		USARTSendDMA("wait_5\r\n");
		l_WaitX(1,10);  
		USARTSendDMA("wait_10\r\n");
		l_WaitX(2,15);  
		USARTSendDMA("wait_15\r\n");
		l_WaitX(3,20);  
		USARTSendDMA("wait_25\r\n");
		l_WaitX(5,25);  
		USARTSendDMA("wait_30\r\n");
		l_WaitX(6,30);  
		USARTSendDMA("wait_35\r\n");
		l_WaitX(7,35);  
		USARTSendDMA("wait_40\r\n");
		/*访问任务3 线程内访问 当前任务被挂起 堵塞住当前任务*/
		l_CallSub(30,LedTask3);
		/*使自己退出线程*/
		fsm_task_off(LedTask);
	}
)


simple_fsm(LedTask2,
	unsigned char pin;
	unsigned char timeon;
	unsigned char timelen; 
)
fsm_initialiser(LedTask2,
	while(1) {
		// l_WaitUntil(8,RX_FLAG_END_LINE);
		// USARTSendDMA(RX_BUF);
		l_WaitX(1,100);
		USARTSendDMA("C_LedTask2.\r\n");
		GPIO_WriteBit(GPIOC,GPIO_Pin_7,(BitAction)!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7));
		if(me.pin == 10) {
			me.pin = 0;
			/*调用了初始化，每次线程都是从开头开始运行*/
			fsm_task_init(LedTask);
			/*新建任务led 新线程开启 不影响当前任务运行，不堵塞当前任务*/
			fsm_task_on(LedTask);
			device *ledx = get_device("led");
			if(ledx != NULL) {
				ledx->write(100);
			}
		} else {
			me.pin++;
		}
		RX_FLAG_END_LINE = 0;
	}
)


void led_set(unsigned char type,...) {
	USARTSendDMA("Led  run.\r\n");
}

simple_fsm(obj_list,)

fsm_initialiser(obj_list,
	/*创建设备*/
	device *led = GET_DAV;
	led->write = &led_set;
	/*注册设备*/
	register_dev_obj("led",led);
	USARTSendDMA("Led  register.\r\n");
	while(1) {
		l_WaitX(1,210);
//		device *ledx = get_device("led");
//		if(ledx != NULL) {
//			ledx->write(100);
//		}
//		uregister_dev_obj("led");
	}
)

//------------------------------------------------------------------------
void runtasks(void){
	/*任务列表*/
	fsm_going(LedTask2);
	fsm_going(LedTask);
	fsm_going(obj_list);
}

int main(int argc, char *argv[]) {
	app_set();
	gpio_init();
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
	
	BKP_DeInit();	//复位备份区域 	
	RCC_LSEConfig(RCC_LSE_Bypass);	//设置外部低速晶振(LSE),使用外设低速晶振
//	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET&&temp<250)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
//	{
//	}
//	if(temp>=250)return 1;//初始化时钟失败,晶振有问题	    
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
	RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟  
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_WaitForSynchro();		//等待RTC寄存器同步  
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_EnterConfigMode();/// 允许配置	
	RTC_SetPrescaler(32767); //设置RTC预分频的值
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_ExitConfigMode(); //退出配置模式  
	//BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//向指定的后备寄存器中写入用户程序数据

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
	RTC_SetCounter(65534);	//设置RTC计数器的值

	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	

//	time_init();
	// Initialize USART
//    usart_dma_init();
//	USARTSendDMA("Hello.\r\nUSART1 is ready.\r\n");
//	
//	 PWR_EnterSTANDBYMode();  

//	MPU6050_I2C_Init();
//	//MPU6050_Initialize();
//	MPU6050_GetDeviceID();
	/*开启任务2*/
	fsm_task_on(LedTask2);
	fsm_task_on(obj_list);
	while(1) {
		/*1ms*/
		if(TimeSec >= 1) {
			TimeSec = 0;
			runtasks();
		}
	}
}




