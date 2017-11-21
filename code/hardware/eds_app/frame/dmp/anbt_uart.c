/*************************************************************************************************************
Բ�㲩ʿС���������2014������Դ��������:
��Դ��������ο�,Բ�㲩ʿ����Դ�����ṩ�κ���ʽ�ĵ���,Ҳ������ʹ�ø�Դ��������ֵ���ʧ����.
�û�������ѧϰ��Ŀ���޸ĺ�ʹ�ø�Դ����.
���û����޸ĸ�Դ����ʱ,�����Ƴ��ò��ְ�Ȩ��Ϣ�����뱣��ԭ������.

������Ϣ������ʹٷ���վwww.etootle.com, �ٷ�����:http://weibo.com/xiaosizhou
**************************************************************************************************************/
#include "anbt_uart.h"
#include "anbt_uart_def.h"

void AnBT_UART1_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = AnBT_USART1_TX;					//Բ�㲩ʿ:����PA9�ܽ�Ϊ����TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 		//Բ�㲩ʿ:���ô���TX�����������ٶ�
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   		//Բ�㲩ʿ:���ô���TXΪ���
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//
  GPIO_InitStructure.GPIO_Pin = AnBT_USART1_RX;					//Բ�㲩ʿ:����PA9�ܽ�Ϊ����RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //Բ�㲩ʿ:���ô���RXΪ����
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	//
	AnBT_Uart1_Send_String("M-1,Init COM Device.",20);
}

void AnBT_UART1_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;
	//
  USART_InitStructure.USART_BaudRate = 9600;									//Բ�㲩ʿ:���ô��ڲ�����Ϊ115200
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //Բ�㲩ʿ:���ô������ݳ���Ϊ8λ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;        //Բ�㲩ʿ:���ô���ֹͣλ����Ϊ1λ
  USART_InitStructure.USART_Parity = USART_Parity_No ;					//Բ�㲩ʿ:���ô�����żУ��Ϊ��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //Բ�㲩ʿ:���ô�������������Ϊ��
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;										//Բ�㲩ʿ:���ô���Ϊ���ͺͽ���ģʽ
  USART_Init(USART1, &USART_InitStructure);			//Բ�㲩ʿ:���ô��ڲ���
  USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);	//Բ�㲩ʿ:��������ж�
  USART_Cmd(USART1, ENABLE);  									//Բ�㲩ʿ:ʹ�ܴ���
}

void AnBT_UART1_NVIC_Configuration(void)				//Բ�㲩ʿ:���ô����ж����ȼ�
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
	
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET)			//Բ�㲩ʿ:�������Ǵ��ڽ����ж�
	{
		com_receive_data=USART_ReceiveData(USART1);     				//Բ�㲩ʿ:���յ����ݡ�
		if(com_receive_data==AnBT_Command_Head)   //Բ�㲩ʿ:������ݿ�ʼ��־
		{
			com_receive_str_index=0;     
			valid_command_was_received=0;			
		}
		else if(com_receive_data==AnBT_Command_Tail)  //Բ�㲩ʿ:���ݽ�����־
		{
			if(com_receive_str_index>1) 
			{			
				for(i=0;i<com_receive_str_index-2;i++) com_data_checksum += com_receive_str_buf[i];    //Բ�㲩ʿ:��������У��ͱ�־
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
					if((com_receive_str_buf[0]=='B')&&(com_receive_str_buf[1]=='8'))	//Բ�㲩ʿ:�����յ���PID����
					{
						for(i=0;i<12;i++) 
						{
							pid_data_buffer[i]=com_receive_str_buf[i+2]-48;
							if(pid_data_buffer[i]>9) pid_data_buffer[i]=0;
						}
						motor_unlock_sign=com_receive_str_buf[14]-48;
					}				
					else if((com_receive_str_buf[0]=='B')&&(com_receive_str_buf[1]=='0'))  //Բ�㲩ʿ:�����յ���ң�����Ƕ����ݺ���������
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
						valid_command_was_received=1;		    //Բ�㲩ʿ:���ݺϷ���־		
					}
				}					
			}
		}
		else
		{
			com_receive_str_buf[com_receive_str_index] = com_receive_data;
			if(com_receive_str_index<AnBT_COM_Buf_Length-1) com_receive_str_index++;			//Բ�㲩ʿ:���ջ����ַ��1
			else com_receive_str_index=0;					//Բ�㲩ʿ:��0���ܻ����ַ,��ֹ�������
		}
		USART_ClearFlag(USART1,USART_FLAG_RXNE);	//Բ�㲩ʿ:����жϱ�־
	}
}

void AnBT_Uart1_Send_Char(unsigned char ascii_code) 		//Բ�㲩ʿ:����һ���ַ�
{
	USART_SendData(USART1,ascii_code);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//Բ�㲩ʿ:�ȴ�ֱ���������
}

void AnBT_Uart1_Send_String(unsigned char* str_buf , unsigned char str_len)		//Բ�㲩ʿ:����һ��ָ�����ȵ��ַ���
{
	unsigned char i;
	if(str_len>AnBT_COM_Buf_Length) str_len=AnBT_COM_Buf_Length;
	AnBT_Uart1_Send_Char(13);																	//Բ�㲩ʿ:���ͻس��ַ�
	AnBT_Uart1_Send_Char(':');  															//Բ�㲩ʿ:�����ַ�:
  for(i=0;i<str_len;i++) AnBT_Uart1_Send_Char(str_buf[i]); 	//Բ�㲩ʿ:�����ַ�:
	AnBT_Uart1_Send_Char('/');																//Բ�㲩ʿ:�����ַ�/
	AnBT_Uart1_Send_Char(13);																	//Բ�㲩ʿ:���ͻس��ַ�
}

void AnBT_Uart1_Send_Num(unsigned char number) 	//Բ�㲩ʿ:����һ���ַ�
{
	unsigned char num_low,num_high;
	num_low=number&0x0f;													//Բ�㲩ʿ:ȡ���ݵ�λ
	num_high=(number&0xf0)>>4;										//Բ�㲩ʿ:ȡ���ݸ�λ
	if(num_high<10)USART_SendData(USART1,num_high+48);
	else USART_SendData(USART1,num_high+55);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//Բ�㲩ʿ:�ȴ�ֱ���������
	if(num_low<10)USART_SendData(USART1,num_low+48);
	else USART_SendData(USART1,num_low+55);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}								//Բ�㲩ʿ:�ȴ�ֱ���������
}

void AnBT_Uart1_Send_Nums(unsigned char* nums_buf , unsigned char nums_len)		//Բ�㲩ʿ:����һ��ָ�����ȵ��ַ���
{
	unsigned char i;
	if(nums_len>AnBT_COM_Buf_Length) nums_len=AnBT_COM_Buf_Length;
	AnBT_Uart1_Send_Char(13);																										//Բ�㲩ʿ:���ͻس��ַ�
	AnBT_Uart1_Send_Char(':');  																								//Բ�㲩ʿ:�����ַ�:
  for(i=0;i<nums_len;i++) AnBT_Uart1_Send_Num(nums_buf[nums_len-i-1]); 				//Բ�㲩ʿ:��������
	AnBT_Uart1_Send_Char('/');																									//Բ�㲩ʿ:�����ַ�/
}

