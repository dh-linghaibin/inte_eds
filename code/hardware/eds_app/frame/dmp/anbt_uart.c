/*************************************************************************************************************
圆点博士小四轴飞行器2014版配套源代码声明:
该源代码仅供参考,圆点博士不对源代码提供任何形式的担保,也不对因使用该源代码而出现的损失负责.
用户可以以学习的目的修改和使用该源代码.
但用户在修改该源代码时,不得移除该部分版权信息，必须保留原版声明.

更多信息，请访问官方网站www.etootle.com, 官方博客:http://weibo.com/xiaosizhou
**************************************************************************************************************/
#include "anbt_uart.h"
#include "anbt_uart_def.h"

void AnBT_UART1_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = AnBT_USART1_TX;					//圆点博士:设置PA9管脚为串口TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 		//圆点博士:设置串口TX最大允许输出速度
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   		//圆点博士:设置串口TX为输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//
  GPIO_InitStructure.GPIO_Pin = AnBT_USART1_RX;					//圆点博士:设置PA9管脚为串口RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //圆点博士:设置串口RX为输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	//
	AnBT_Uart1_Send_String("M-1,Init COM Device.",20);
}

void AnBT_UART1_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;
	//
  USART_InitStructure.USART_BaudRate = 9600;									//圆点博士:设置串口波特率为115200
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //圆点博士:设置串口数据长度为8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;        //圆点博士:设置串口停止位长度为1位
  USART_InitStructure.USART_Parity = USART_Parity_No ;					//圆点博士:设置串口奇偶校验为无
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //圆点博士:设置串口数据流控制为无
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;										//圆点博士:设置串口为发送和接收模式
  USART_Init(USART1, &USART_InitStructure);			//圆点博士:设置串口参数
  USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);	//圆点博士:允许接收中断
  USART_Cmd(USART1, ENABLE);  									//圆点博士:使能串口
}

void AnBT_UART1_NVIC_Configuration(void)				//圆点博士:设置串口中断优先级
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = 37;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void AnBT_UART1_Interrupt(void)
{
	u8 com_receive_data=0;
	u8 com_receive_data_checksum=0;
	u8 com_receive_data_checksum_low,com_receive_data_checksum_high;
	u8 com_data_checksum=0;
	u8 i;
	
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET)			//圆点博士:发生的是串口接收中断
	{
		com_receive_data=USART_ReceiveData(USART1);     				//圆点博士:接收到数据。
		if(com_receive_data==AnBT_Command_Head)   //圆点博士:检查数据开始标志
		{
			com_receive_str_index=0;     
			valid_command_was_received=0;			
		}
		else if(com_receive_data==AnBT_Command_Tail)  //圆点博士:数据结束标志
		{
			if(com_receive_str_index>1) 
			{			
				for(i=0;i<com_receive_str_index-2;i++) com_data_checksum += com_receive_str_buf[i];    //圆点博士:计算数据校验和标志
				com_receive_data_checksum_low=com_receive_str_buf[com_receive_str_index-1];
				com_receive_data_checksum_high=com_receive_str_buf[com_receive_str_index-2];				
				if(com_receive_data_checksum_low>58) com_receive_data_checksum_low-=55;
				else com_receive_data_checksum_low-=48;
				if(com_receive_data_checksum_high>58) com_receive_data_checksum_high-=55;
				else com_receive_data_checksum_high-=48;	
				com_receive_data_checksum=((com_receive_data_checksum_high<<4)&0xf0)|(com_receive_data_checksum_low&0x0f);
				com_data_checksum=com_data_checksum+com_receive_data_checksum;
				//	
				if(com_data_checksum==0) 
				{					
					if((com_receive_str_buf[0]=='B')&&(com_receive_str_buf[1]=='8'))	//圆点博士:所接收的是PID数据
					{
						for(i=0;i<12;i++) 
						{
							pid_data_buffer[i]=com_receive_str_buf[i+2]-48;
							if(pid_data_buffer[i]>9) pid_data_buffer[i]=0;
						}
						motor_unlock_sign=com_receive_str_buf[14]-48;
					}				
					else if((com_receive_str_buf[0]=='B')&&(com_receive_str_buf[1]=='0'))  //圆点博士:所接收的是遥控器角度数据和油门数据
					{						
						for(i=0;i<2;i++)
						{
							if(com_receive_str_buf[i+2]>58) com_receive_str_buf[i+2]-=55;
							else com_receive_str_buf[i+2]-=48;				
						}
						gas_data_buffer=((com_receive_str_buf[2]<<4)&0xf0)|(com_receive_str_buf[3]&0x0f);
						//
						for(i=0;i<2;i++)
						{
							if(com_receive_str_buf[i+4]>58) com_receive_str_buf[i+4]-=55;
							else com_receive_str_buf[i+4]-=48;
						}
						pitch_data_buffer=((com_receive_str_buf[4]<<4)&0xf0)|(com_receive_str_buf[5]&0x0f);
						//
						for(i=0;i<2;i++)
						{
							if(com_receive_str_buf[i+6]>58) com_receive_str_buf[i+6]-=55;
							else com_receive_str_buf[i+6]-=48;
						}
						roll_data_buffer=((com_receive_str_buf[6]<<4)&0xf0)|(com_receive_str_buf[7]&0x0f);
						//
						for(i=0;i<2;i++)
						{
							if(com_receive_str_buf[i+8]>58) com_receive_str_buf[i+8]-=55;
							else com_receive_str_buf[i+8]-=48;
						}
						yaw_data_buffer=((com_receive_str_buf[8]<<4)&0xf0)|(com_receive_str_buf[9]&0x0f);
					}
					else
					{
						valid_command_was_received=1;		    //圆点博士:数据合法标志		
					}
				}					
			}
		}
		else
		{
			com_receive_str_buf[com_receive_str_index] = com_receive_data;
			if(com_receive_str_index<AnBT_COM_Buf_Length-1) com_receive_str_index++;			//圆点博士:接收缓冲地址加1
			else com_receive_str_index=0;					//圆点博士:清0接受缓冲地址,防止数组溢出
		}
		USART_ClearFlag(USART1,USART_FLAG_RXNE);	//圆点博士:清除中断标志
	}
}

void AnBT_Uart1_Send_Char(unsigned char ascii_code) 		//圆点博士:发送一个字符
{
	USART_SendData(USART1,ascii_code);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//圆点博士:等待直到发送完成
}

void AnBT_Uart1_Send_String(unsigned char* str_buf , unsigned char str_len)		//圆点博士:发送一个指定长度的字符串
{
	unsigned char i;
	if(str_len>AnBT_COM_Buf_Length) str_len=AnBT_COM_Buf_Length;
	AnBT_Uart1_Send_Char(13);																	//圆点博士:发送回车字符
	AnBT_Uart1_Send_Char(':');  															//圆点博士:发送字符:
  for(i=0;i<str_len;i++) AnBT_Uart1_Send_Char(str_buf[i]); 	//圆点博士:发送字符:
	AnBT_Uart1_Send_Char('/');																//圆点博士:发送字符/
	AnBT_Uart1_Send_Char(13);																	//圆点博士:发送回车字符
}

void AnBT_Uart1_Send_Num(unsigned char number) 	//圆点博士:发送一个字符
{
	unsigned char num_low,num_high;
	num_low=number&0x0f;													//圆点博士:取数据低位
	num_high=(number&0xf0)>>4;										//圆点博士:取数据高位
	if(num_high<10)USART_SendData(USART1,num_high+48);
	else USART_SendData(USART1,num_high+55);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//圆点博士:等待直到发送完成
	if(num_low<10)USART_SendData(USART1,num_low+48);
	else USART_SendData(USART1,num_low+55);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//圆点博士:等待直到发送完成
}

void AnBT_Uart1_Send_Nums(unsigned char* nums_buf , unsigned char nums_len)		//圆点博士:发送一个指定长度的字符串
{
	unsigned char i;
	if(nums_len>AnBT_COM_Buf_Length) nums_len=AnBT_COM_Buf_Length;
	AnBT_Uart1_Send_Char(13);																										//圆点博士:发送回车字符
	AnBT_Uart1_Send_Char(':');  																								//圆点博士:发送字符:
  for(i=0;i<nums_len;i++) AnBT_Uart1_Send_Num(nums_buf[nums_len-i-1]); 				//圆点博士:发送数字
	AnBT_Uart1_Send_Char('/');																									//圆点博士:发送字符/
}

