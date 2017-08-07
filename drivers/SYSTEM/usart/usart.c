/*****************************************************************************/
/* File      : usart.c                                                        */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/

#include "sys.h"
#include "usart.h"	
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"
#include "delay.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_RCC                    RCC_APB2Periph_GPIOB
#define WIFI_GPIO                   GPIOB
#define WIFI_PIN                    (GPIO_Pin_15)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
unsigned short packetlen(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//WIFI发送函数 
int WIFI_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 
    USART_SendData(USART2,(uint8_t)ch);
	}
	return index;
}

//USART1发送函数 
int UART1_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);
	}
	return index;
}

void uart1_init(u32 bound){
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能串口时钟USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
 
   //USART1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断

    USART_Cmd(USART1, ENABLE);                    //使能串口 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(WIFI_GPIO, WIFI_PIN);

}


unsigned char USART1_RecvData[USART1_REC_LEN] = {'\0'};
unsigned char USART1_Recv_Event = 0;

unsigned char USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
unsigned short USART1Cur = 0;		//当前采值位置
unsigned short USART1_PackLen = 0;
eRecvSM Usart1eStateMachine = EN_RECV_ST_GET_HEAD;	//数据采集状态机
unsigned short USART1_pos = 0;				//数据解析位置
unsigned short USART1_mvsize = 0;


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART1_RX_BUF[USART1Cur] = USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		//SEGGER_RTT_printf(0, "%x %c\n",USART1_RX_BUF[USART1Cur],USART1_RX_BUF[USART1Cur]);
		USART1Cur +=1;
		if(USART1Cur >= USART1_REC_LEN)
		{
			//SEGGER_RTT_printf(0, "%d \n",USART1Cur);
			USART1Cur = 0;
		}

		

	}
} 

void USART1_GetEvent(int *messageLen)
{
	  USART1_pos = 0;
		//receive start character
		if(Usart1eStateMachine == EN_RECV_ST_GET_HEAD)    //接收报文头部
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(USART1_pos < USART1Cur)
      {
				TIM4_Int_Deinit();
				USART1_mvsize = USART1Cur - USART1_pos;		//当前第几个字节
				if(1 == USART1_mvsize)   //'A'
				{
						if(USART1_RX_BUF[0] != 'E')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
				}
				
				if(2 == USART1_mvsize)   //'P'
				{
						if(USART1_RX_BUF[1] != 'C')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
				}	
				
				if(3 == USART1_mvsize)   //'S'
				{
						if(USART1_RX_BUF[2] != 'U')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
				}
				
				if(5 == USART1_mvsize)   //接收版本号完毕
				{
					//SEGGER_RTT_printf(0, "ECU11\n");
					Usart1eStateMachine = EN_RECV_ST_GET_LEN;

					TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				
				TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				
				USART1_pos++;
			}
		}
		
		//receive data length
		if(Usart1eStateMachine == EN_RECV_ST_GET_LEN)
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_LEN\n");
			while(USART1_pos < USART1Cur)
      {
				TIM4_Int_Deinit();
				USART1_mvsize = USART1Cur - USART1_pos;		//当前第几个字节
				if(9 == USART1_mvsize)   //接收数据长度结束
				{
					USART1_PackLen = packetlen(&USART1_RX_BUF[5]);
					//SEGGER_RTT_printf(0, "LENGTH : %d\n",USART1_PackLen);
					//计算长度
					Usart1eStateMachine = EN_RECV_ST_GET_DATA;

					TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				USART1_pos++;
			}
		}
		
		//Continue to receive data
		if(Usart1eStateMachine == EN_RECV_ST_GET_DATA)
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_DATA\n");
			while(USART1_pos < USART1Cur)
      {
				TIM4_Int_Deinit();
				USART1_mvsize = USART1Cur - USART1_pos;		//当前第几个字节
				if((USART1_PackLen - 3) == USART1_mvsize)   //接收数据长度结束
				{
					Usart1eStateMachine = EN_RECV_ST_GET_END;

					TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				USART1_pos++;
			}
		}
		
		
		
		//receive END
		if(Usart1eStateMachine == EN_RECV_ST_GET_END)
		{
		
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END\n");
			while(USART1_pos < USART1Cur)
      {
				TIM4_Int_Deinit();
				USART1_mvsize = USART1Cur - USART1_pos;		//当前第几个字节
				if((USART1_PackLen - 2) == USART1_mvsize)   //'A'
				{
						if(USART1_RX_BUF[USART1_PackLen - 3] != 'E')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
				}
				
				if((USART1_PackLen - 1) == USART1_mvsize)   //'P'
				{
						if(USART1_RX_BUF[USART1_PackLen - 2] != 'N')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
				}	
				
				if((USART1_PackLen) == USART1_mvsize)   //'S'
				{
						if(USART1_RX_BUF[USART1_PackLen - 1] != 'D')
						{
							USART1Cur = 0;
							USART1_pos = 0;
							Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						
						//报文接收完毕
						//进行完毕的相应操作
						//将采集成功的数据复制到成功数组
						memset(USART1_RecvData,0x00,USART1_REC_LEN);
						memcpy(USART1_RecvData,USART1_RX_BUF,USART1_PackLen);
						*messageLen = USART1_PackLen;
						USART1_RecvData[USART1_PackLen] = '\0';
						USART1_Recv_Event = 1;
						//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",USART1_RecvData);
						Usart1eStateMachine = EN_RECV_ST_GET_HEAD;
						USART1Cur = 0;
						USART1_pos = 0;
						
						TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
						break;
				}
				
				TIM4_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				
				USART1_pos++;
			}
		}
}


 
#if EN_USART2_RX   //如果使能了接收
//初始化IO 串口1 
//bound:波特率
void uart2_init(u32 bound){
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能串口时钟USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
 	USART_DeInit(USART2);  //复位串口1
	 //USART2_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART2_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口
#if EN_USART2_RX		  //如果使能了接收  
   //USART2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART2, ENABLE);                    //使能串口 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(WIFI_GPIO, WIFI_PIN);

}

//串口2中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	

unsigned char WIFI_RecvData[USART_REC_LEN] = {'\0'};
unsigned char WIFI_Recv_Event = 0;

unsigned char USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
unsigned short Cur = 0;		//当前采值位置
unsigned short PackLen = 0;
eRecvSM eStateMachine = EN_RECV_ST_GET_A;	//数据采集状态机
unsigned short pos = 0;				//数据解析位置
unsigned short mvsize = 0;

void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
		//SEGGER_RTT_printf(0, "[%d] : %x %c\n",Cur,USART_RX_BUF[Cur],USART_RX_BUF[Cur]);
		Cur +=1;
		if(Cur >=USART_REC_LEN)
		{
			//SEGGER_RTT_printf(0, "%d \n",Cur);
			Cur = 0;
		}
	}
} 

void WIFI_GetEvent(int *messageLen,unsigned char *ID)
{
	  pos = 0;
		
		//接收A报文头部
		if(eStateMachine == EN_RECV_ST_GET_A)
		{
			while(pos < Cur)
      {
				mvsize = Cur - pos;		//当前第几个字节
				if(1 == mvsize)   //'a'
				{
						if(USART_RX_BUF[0] != 'a')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}else
						{
							//SEGGER_RTT_printf(0, "a\n");
							delay_ms(2);
							eStateMachine = EN_RECV_ST_GET_ID;
							break;
						}
				}
				pos++;
			}
		}		
	
		//接收ID
		if(eStateMachine == EN_RECV_ST_GET_ID)
		{
      while(pos < Cur)
      {
				//TIM3_Int_Deinit();

				if(2 == pos)
				{
					ID[0] = USART_RX_BUF[1];
				}
				
				if(3 == pos)
				{
					ID[1] = USART_RX_BUF[2];
				}	
				
				if(4 == pos)
				{
					ID[2] = USART_RX_BUF[3];
				}
				
				if(5 == pos) 
				{
					ID[3] = USART_RX_BUF[4];
				}
				
				if(6 == pos)
				{
					ID[4] = USART_RX_BUF[5];
				}
				
				if(7 == pos)
				{
					ID[5] = USART_RX_BUF[6];
				}
				
				if(8 == pos)   
				{
					ID[6] = USART_RX_BUF[7];
				}
				
				if(9 == pos)   //接收版本号完毕
				{
					//SEGGER_RTT_printf(0, "ID\n");
					ID[7] = USART_RX_BUF[8];
					eStateMachine = EN_RECV_ST_GET_HEAD;
					//SEGGER_RTT_printf(0, "ID111 %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);

					break;
				}
								
				pos++;
			}
		}			
	
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_HEAD)    //接收报文头部
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(pos < Cur)
      {
				//TIM3_Int_Deinit();
				mvsize = Cur - pos;		//当前第几个字节
				if(10 == mvsize)   //'A'
				{
						if(USART_RX_BUF[9] != 'A')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if(11 == mvsize)   //'P'
				{
						if(USART_RX_BUF[10] != 'P')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}	
				
				if(12 == mvsize)   //'S'
				{
						if(USART_RX_BUF[11] != 'S')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if(14 == mvsize)   //接收版本号完毕
				{
					//SEGGER_RTT_printf(0, "APS11\n");
					eStateMachine = EN_RECV_ST_GET_LEN;

					break;
				}
								
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_LEN)
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_LEN\n");
			while(pos < Cur)
      {
				//判断是否有a出现  如果出现了判断后面8个字节
				mvsize = Cur - pos;		//当前第几个字节
				if(18 == mvsize)   //接收数据长度结束
				{
					PackLen = (packetlen(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "LENGTH11111 : %d\n",PackLen);
					//计算长度
					eStateMachine = EN_RECV_ST_GET_DATA;
					delay_ms(10);
					TIM3_Int_Init(299,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器

					break;
				}
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_DATA)
		{
			pos = 0;
			while(pos < Cur)
      {

				if((PackLen - 3) == pos)   //接收数据长度结束
				{
					eStateMachine = EN_RECV_ST_GET_END;
					//delay_ms(10);
					//TIM3_Int_Init(299,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				pos++;
			}
		}		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_END)
		{
			pos = 0;
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END\n");
			while(pos < Cur)
      {
				mvsize = Cur - pos;		//当前第几个字节
				if((PackLen - 2) == mvsize)   //'A'
				{
						if(USART_RX_BUF[PackLen - 3] != 'E')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if((PackLen - 1) == mvsize)   //'P'
				{
						if(USART_RX_BUF[PackLen - 2] != 'N')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}	
				
				if((PackLen) == mvsize)   //'S'
				{
						if(USART_RX_BUF[PackLen - 1] != 'D')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						
						//SEGGER_RTT_printf(0, "ID %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
						
						//报文接收完毕
						//进行完毕的相应操作
						//将采集成功的数据复制到成功数组
						memset(WIFI_RecvData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvData,&USART_RX_BUF[9],(PackLen-9));
						//解析数据，去掉多包问题
						*messageLen = PackLen-9;
						
						WIFI_RecvData[*messageLen] = '\0';
						WIFI_Recv_Event = 1;
						//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvData);
						eStateMachine = EN_RECV_ST_GET_A;
						Cur = 0;
						pos = 0;		
						TIM3_Int_Deinit();
						break;
				}
								
				pos++;
			}
		}
		
}

void clear_WIFI(void)
{
	//TIM3_Int_Deinit();
	eStateMachine = EN_RECV_ST_GET_A;
	Cur = 0;
}

//进入AT模式
int AT(void)
{
	clear_WIFI();
	//先向模块写入"+++"然后再写入"a" 写入+++返回"a" 写入"a"返回+ok
	WIFI_SendData("+++", 3);
	//获取到a
	delay_ms(350);
	if(Cur <1)
	{
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"a",1))
		{
			return -1;
		}
	}
	
	//接着发送a
	clear_WIFI();
	WIFI_SendData("a", 1);
	delay_ms(350);
	if(Cur < 3)
	{
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT :a+ok\n");
	clear_WIFI();
	return 0;
}


//切换回原来的工作模式    OK
int AT_ENTM(void)
{

	clear_WIFI();
	//发送"AT+ENTM\n",返回+ok
	WIFI_SendData("AT+ENTM\n", 8);
	delay_ms(300);
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[9],"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+ENTM :+ok\n");
	clear_WIFI();
	return 0;
	
}

int AT_Z(void)
{

	clear_WIFI();
	//发送"AT+Z\n",返回+ok
	WIFI_SendData("AT+Z\n", 5);
	delay_ms(300);
	if(Cur < 6)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[6],"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+Z :+ok\n");
	clear_WIFI();
	return 0;
	
}

//设置WIFI SSID

int AT_WAP(char *ECUID12)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAP=11BGN,ECU_R_%s,Auto\n",ECUID12);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAP :+ok\n");
	clear_WIFI();
	return 0;
}

//设置WIFI密码
int AT_WAKEY(char *NewPasswd)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAKEY=WPA2PSK,AES,%s\n",NewPasswd);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY :+ok\n");
	clear_WIFI();
	return 0;
}

//设置WIFI密码
int AT_WAKEY_Clear(void)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//发送"AT+WAKEY\n",返回+ok
	sprintf(AT,"AT+WAKEY=OPEN,NONE\n");
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY Clear :+ok\n");
	clear_WIFI();
	return 0;
}



int WIFI_ChangePasswd(char *NewPasswd)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_WAKEY(NewPasswd);
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();;
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;
}

int WIFI_Reset(void)
{
	GPIO_ResetBits(WIFI_GPIO, WIFI_PIN);
	
	delay_ms(1000);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	return 0;
}

int WIFI_SoftReset(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);	
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) 
	{
		WIFI_Reset();	
		return -1;
	}
	
	return 0;
}

int WIFI_ClearPasswd(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret = AT_WAKEY_Clear();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}


int WIFI_Test(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0)return 0;
		}
	
		return -1;
	}	
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();
		if(ret == 0) return 0;
	}
	return -1;
}


int WIFI_Factory(char *ECUID12)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret = AT_WAP(ECUID12);
		ret = AT_WAKEY("88888888");
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}


#endif	

