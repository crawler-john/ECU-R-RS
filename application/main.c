/*****************************************************************************/
/* File      : main.c                                                        */
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
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "Flash_24L512.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "RFM300H.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "file.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
char ECUID12[13] = {'\0'};
char ECUID6[7] = {'\0'};
char Signal_Level = 0;
char Signal_Channel[3] = {'\0'};
char Channel_char = 0;
char IO_Init_Status = 0;			//IO初始状态
char ver = 0;						//优化器版本号
int validNum = 0;				//当前有效台数
int curSequence = 0;		//心跳轮训机器号
inverter_info inverterInfo[MAXINVERTERCOUNT] = {'\0'};
int Data_Len = 0,Command_Id = 0,ResolveFlag = 0,messageLen = 0,messageUsart1Len = 0;
int UART1_Data_Len = 0,UART1_Command_Id = 0,UART1_ResolveFlag = 0;
unsigned char ID[9] = {'\0'};

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int main(void)
{	
	int ret = 0;
//	unsigned reboot_num = 0;
	
	delay_init();	    	 					//延时函数初始化	
	NVIC_Configuration(); 				//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	I2C_Init();										//FLASH 芯片初始化
	LED_init();										//LED灯初始化
	EXTIX_Init();									//恢复出厂设置IO中断初始化
	KEY_Init();										//恢复出厂设置按键初始化
	RFM_init();
	CMT2300_init();
	uart1_init(57600);
	uart2_init(57600);							//串口初始化
	TIM2_Int_Init(4999,7199);    //心跳包超时事件定时器初始化
	rt_hw_watchdog_init();
	SEGGER_RTT_printf(0, "init OK \n");
	init_ecu();										//初始化ECU
	init_inverter(inverterInfo);	//初始化逆变器
	
//	Read_rebootNum(&reboot_num);
//	Write_rebootNum((reboot_num+1));	
//	SEGGER_RTT_printf(0,"reboot num:%d\n",reboot_num);
#if 1
	while(1)
	{	
		//检测WIFI事件
		process_WIFIEvent();
		
		//检测USART1事件
		USART1_GetEvent(&messageUsart1Len);
		if(USART1_Recv_Event == 1)
		{
//			SEGGER_RTT_printf(0,"reboot num:%d\n",reboot_num);
			SEGGER_RTT_printf(0,"USART1_Recv_Event start\n");
			SEGGER_RTT_printf(0, "USART1_Recv_Event %s\n",USART1_RecvData);
			process_UART1Event();
			USART1_Recv_Event = 0;
			SEGGER_RTT_printf(0,"USART1_Recv_Event end\n");
		}
		//检测按键事件
		if(KEY_FormatWIFI_Event == 1)
		{
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event start\n");
			process_KEYEvent();
			KEY_FormatWIFI_Event = 0;
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event end\n");
		}
		
		//WIFI复位事件
		if(WIFI_RST_Event == 1)
		{
			SEGGER_RTT_printf(0,"WIFI_RST_Event start\n");
			ret = process_WIFI_RST();
			if(ret == 0)
				WIFI_RST_Event = 0;
			SEGGER_RTT_printf(0,"WIFI_RST_Event end\n");
		}
		
		//判断是否有433模块心跳超时事件
		if(COMM_Timeout_Event == 1)
		{
			SEGGER_RTT_printf(0,"COMM_Timeout_Event start\n");
			process_HeartBeatEvent();			
			kickwatchdog();
			COMM_Timeout_Event = 0;
			SEGGER_RTT_printf(0,"COMM_Timeout_Event end\n");
		}
				
	}	
#endif
 }

