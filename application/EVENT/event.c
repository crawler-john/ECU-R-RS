/*****************************************************************************/
/* File      : event.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-09 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "event.h"
#include "RFM300H.h"
#include "file.h"
#include "string.h"
#include "usart.h"
#include "appcomm.h"
#include "SEGGER_RTT.h"
#include "inverter.h"
#include "USART1Comm.h"
#include "delay.h"
#include "led.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define PERIOD_NUM			3600

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
unsigned short comm_failed_Num = 0;
unsigned short pre_heart_rate;


typedef enum
{ 
    HARDTEST_TEST_ALL    	= 0,		//接收数据头
    HARDTEST_TEST_EEPROM  = 1,	//接收数据长度   其中数据部分的长度为接收到长度减去12个字节
    HARDTEST_TEST_WIFI  	= 2,	//接收数据部分数据
    HARDTEST_TEST_433  		= 3
} eHardWareID;// receive state machin

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int ResolveWifiPasswd(char *oldPasswd,int *oldLen,char *newPasswd,int *newLen,char *passwdstring)
{
	*oldLen = (passwdstring[0]-'0')*10+(passwdstring[1]-'0');
	memcpy(oldPasswd,&passwdstring[2],*oldLen);
	*newLen = (passwdstring[2+(*oldLen)]-'0')*10+(passwdstring[3+(*oldLen)]-'0');
	memcpy(newPasswd,&passwdstring[4+*oldLen],*newLen);
	
	return 0;
}

//硬件测试   返回值是测试的错误码
int HardwareTest(char testItem)
{
	char testWrite[10] = "YUNENG APS";
	char testRead[10] = {'\0'};

	switch(testItem)
	{
		case HARDTEST_TEST_ALL:
			//测试EEPROM读写
			Write_Test(testWrite,10);				//测试
			Read_Test(testRead,10);
			if(memcmp(testWrite,testRead,10))
			{
				return 1;
			}
			//测试WIFI模块
			if(0 != WIFI_Test())
			{
				return 2;
			}
			
			//测试433模块

			
			break;
		
		case HARDTEST_TEST_EEPROM:
			//测试EEPROM读写
			break;
		
		case HARDTEST_TEST_WIFI:
			//测试WIFI模块
			if(0 != WIFI_Test())
			{
				return 2;
			}
			break;
		
		case HARDTEST_TEST_433:
			//测试433模块

			break;
				
	}
	return 0;
}


void process_WIFIEvent(void)
{
	//检测WIFI事件
	WIFI_GetEvent(&messageLen,ID);
	//判断是否有WIFI接收事件
	if(WIFI_Recv_Event == 1)
	{
		process_WIFI(ID);
		WIFI_Recv_Event = 0;
	}

}


//心跳事件处理
void process_HeartBeatEvent(void)
{
	int ret = 0;
	//SEGGER_RTT_printf(0, "COMM_Timeout_Event \n");
	//发送心跳包
	if(	vaildNum >0	)
	{
		//SEGGER_RTT_printf(0, "RFM300_Heart_Beat %02x%02x%02x%02x%02x%02x\n",inverterInfo[curSequence].uid[0],inverterInfo[curSequence].uid[1],inverterInfo[curSequence].uid[2],inverterInfo[curSequence].uid[3],inverterInfo[curSequence].uid[4],inverterInfo[curSequence].uid[5]);
		
		process_WIFIEvent();
		
		//先保存上一轮的心跳
		pre_heart_rate = inverterInfo[curSequence].heart_rate;
		ret = RFM300_Heart_Beat(ECUID6,(char *)inverterInfo[curSequence].uid,(status_t *)&inverterInfo[curSequence].status,&inverterInfo[curSequence].heart_rate,&inverterInfo[curSequence].off_times,&ver);
	
		process_WIFIEvent();

		if(ret == 0)	//发送心跳包失败
		{
			//查看绑定标志位，如果绑定未成功，尝试绑定。
			if(inverterInfo[curSequence].status.bind_status != 1)
			{
				ret = RFM300_Bind_Uid(ECUID6,(char *)inverterInfo[curSequence].uid,0,0);
				
				process_WIFIEvent();

				//SEGGER_RTT_printf(0, "RFM300_Bind_Uid %02x%02x%02x%02x%02x%02x\n",inverterInfo[curSequence].uid[0],inverterInfo[curSequence].uid[1],inverterInfo[curSequence].uid[2],inverterInfo[curSequence].uid[3],inverterInfo[curSequence].uid[4],inverterInfo[curSequence].uid[5]);
				if(ret == 1)
				{
					if(Write_UID_Bind(0x01,(curSequence+1)) == 0)
					{
						inverterInfo[curSequence].status.bind_status = 1;
						inverterInfo[curSequence].status.heart_Failed_times = 0;
					}
					
				}
			}
			inverterInfo[curSequence].status.heart_Failed_times++;
			if(inverterInfo[curSequence].status.heart_Failed_times >= 3)
			{
				inverterInfo[curSequence].status.heart_Failed_times = 3;
				inverterInfo[curSequence].status.mos_status = 0;
			}
				
			//通信失败，失败次数++
			comm_failed_Num ++;
		}else	//发送心跳包成功
		{
			//通信失败，失败次数++
			comm_failed_Num  = 0;
			inverterInfo[curSequence].status.heart_Failed_times = 0;
			//心跳成功  判断当前系统信道和当前RSD2最后一次通信的信道是否不同   不同则更改之
			if(Channel_char != inverterInfo[curSequence].channel)
			{
					if(Write_UID_Channel(Channel_char,(curSequence+1)) == 0)
					{
						SEGGER_RTT_printf(0, "change Channel %02x%02x%02x%02x%02x%02x  Channel_char:%d   inverterInfo[curSequence].channel:%d\n",inverterInfo[curSequence].uid[0],inverterInfo[curSequence].uid[1],inverterInfo[curSequence].uid[2],inverterInfo[curSequence].uid[3],inverterInfo[curSequence].uid[4],inverterInfo[curSequence].uid[5],Channel_char,inverterInfo[curSequence].channel);

						inverterInfo[curSequence].channel = Channel_char;
					}
			}

			//心跳成功 判断是否需要关闭或者打开心跳功能
			if(inverterInfo[curSequence].status.function_status == 1)	//心跳功能打开
			{
				if(IO_Init_Status == 0)		//需要关闭心跳功能
				{
					RFM300_IO_Init(ECUID6,(char *)inverterInfo[curSequence].uid,0x02,(status_t *)&inverterInfo[curSequence].status,&inverterInfo[curSequence].heart_rate,&inverterInfo[curSequence].off_times,&ver);
				}
			}else					//心跳功能关闭
			{
				if(IO_Init_Status == 1)		//需要打开心跳功能
				{
					RFM300_IO_Init(ECUID6,(char *)inverterInfo[curSequence].uid,0x01,(status_t *)&inverterInfo[curSequence].status,&inverterInfo[curSequence].heart_rate,&inverterInfo[curSequence].off_times,&ver);
				}
			}

			process_WIFIEvent();

			//如果当前一轮心跳小于上一轮心跳,表示重启
			if(inverterInfo[curSequence].heart_rate < pre_heart_rate)
			{
				//当前一轮重启次数+1
				if(inverterInfo[curSequence].restartNum < 255)
					inverterInfo[curSequence].restartNum++;
			}
			
		}

		process_WIFIEvent();

		
		//在绑定成功的情况下才能改信道
		//if(inverterInfo[curSequence].bind_status == 1)
		{
			//查看是否需要改变信道
			if(Channel_char != inverterInfo[curSequence].channel)
			{
				//先变更到RSD2的信道 
				setChannel(inverterInfo[curSequence].channel);
				
				//发送更改信道报文
				ret = RFM300_Bind_Uid(ECUID6,(char *)inverterInfo[curSequence].uid,Channel_char,0);
				if(ret == 1)	//设置信道成功
				{
					if(Write_UID_Channel(Channel_char,(curSequence+1)) == 0)
					{
						SEGGER_RTT_printf(0, "change Channel %02x%02x%02x%02x%02x%02x  Channel_char:%d   inverterInfo[curSequence].channel:%d\n",inverterInfo[curSequence].uid[0],inverterInfo[curSequence].uid[1],inverterInfo[curSequence].uid[2],inverterInfo[curSequence].uid[3],inverterInfo[curSequence].uid[4],inverterInfo[curSequence].uid[5],Channel_char,inverterInfo[curSequence].channel);
						inverterInfo[curSequence].channel = Channel_char;
					}
				}
				
				//更改到系统信道
				setChannel(Channel_char);
			}		
		}
		
		curSequence++;
		if(curSequence >= vaildNum)		//当轮训的序号大于最后一台时，更换到第0台
		{
			curSequence = 0;
		}
		
		//连续通讯不上1小时   表示关机状态
		if(comm_failed_Num >= PERIOD_NUM)
		{
			//SEGGER_RTT_printf(0, "comm_failed_Num:%d   \n",comm_failed_Num);
			for(curSequence = 0;curSequence < vaildNum;curSequence++)
			{
				inverterInfo[curSequence].restartNum = 0;
			}
			curSequence = 0;
		}
		
				
	}
}

//WIFI事件处理
void process_WIFI(unsigned char * ID)
{
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		switch(Command_Id)
		{
			case COMMAND_BASEINFO:				//获取基本信息			APS11001401END
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_BASEINFO,WIFI_RecvData);
				//获取基本信息
				//获取信号强度
				Signal_Level = ReadRssiValue(1);
				APP_Response_BaseInfo(ID,ECUID12,VERSION_ECU_RS,Signal_Level,Signal_Channel,5,SOFEWARE_VERSION);
				break;
					
			case COMMAND_SYSTEMINFO:			//获取系统信息			APS11002602406000000009END
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SYSTEMINFO,WIFI_RecvData);
				//先对比ECUID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ECUID12,12))
				{	//匹配成功进行相应的操作
					SEGGER_RTT_printf(0, "COMMAND_SYSTEMINFO  Mapping\n");
					APP_Response_SystemInfo(ID,0x00,inverterInfo,vaildNum);
				}	else
				{	//不匹配
					SEGGER_RTT_printf(0, "COMMAND_SYSTEMINFO   Not Mapping");
					APP_Response_SystemInfo(ID,0x01,inverterInfo,0);
				}
				break;
					
			case COMMAND_SETNETWORK:			//设置组网					APS11003503406000000009END111111END
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SETNETWORK,WIFI_RecvData);
				//先对比ECUID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ECUID12,12))
				{	//匹配成功进行相应的操作
					int AddNum = 0;
					
							
					AddNum = (messageLen - 29)/6;
					SEGGER_RTT_printf(0, "COMMAND_SETNETWORK   Mapping   %d\n",AddNum);
					//将数据写入EEPROM
					add_inverter(inverterInfo,AddNum,(char *)&WIFI_RecvData[26]);
					APP_Response_SetNetwork(ID,0x00);
					init_inverter(inverterInfo);
					//进行一些绑定操作
					LED_off();
				}	else
				{	//不匹配
					APP_Response_SetNetwork(ID,0x01);
					SEGGER_RTT_printf(0, "COMMAND_SETNETWORK   Not Mapping");
				}
				break;
						
			case COMMAND_SETCHANNEL:			//设置信道					APS11003104406000000009END22END
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SETCHANNEL,WIFI_RecvData);
				if(!memcmp(&WIFI_RecvData[11],ECUID12,12))
				{	//匹配成功进行相应的操作
					//获取新的信道
					memcpy(Signal_Channel,&WIFI_RecvData[26],2);
					Write_CHANNEL(Signal_Channel);		//写入信道
					//获取信号强度
					Signal_Level = ReadRssiValue(1);
					APP_Response_SetChannel(ID,0x00,Signal_Channel,Signal_Level);
					//将其转换为1个字节
					Channel_char = (Signal_Channel[0]-'0')*10+(Signal_Channel[1]-'0');
					//改变自己的信道
					setChannel(Channel_char);
					SEGGER_RTT_printf(0, "COMMAND_SETCHANNEL  Signal_Channel:%s Channel_char%d\n",Signal_Channel,Channel_char);
					
				}	else
				{	//不匹配
					APP_Response_SetChannel(ID,0x01,NULL,NULL);
				}
				break;
						
			case COMMAND_SETWIFIPASSWORD:	//设置WIFI密码	OK		APS11004905406000000009END08123456780887654321END
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SETWIFIPASSWORD,WIFI_RecvData);
				//先对比ECUID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ECUID12,12))
				{	//匹配成功进行相应的操作
					char OldPassword[100] = {'\0'};
					char NewPassword[100] = {'\0'};
					char EEPROMPasswd[100] = {'\0'};
					int oldLen,newLen;
							
					ResolveWifiPasswd(OldPassword,&oldLen,NewPassword,&newLen,(char *)&WIFI_RecvData[26]);
							
					SEGGER_RTT_printf(0, "COMMAND_SETWIFIPASSWORD %d %s %d %s\n",oldLen,OldPassword,newLen,NewPassword);
							
					//读取旧密码，如果旧密码相同，设置新密码
					Read_WIFI_PW(EEPROMPasswd,100);
							
					if(!memcmp(EEPROMPasswd,OldPassword,oldLen))
					{
						APP_Response_SetWifiPassword(ID,0x00);
						WIFI_ChangePasswd(NewPassword);
						Write_WIFI_PW(NewPassword,newLen);
					}else
					{
						APP_Response_SetWifiPassword(ID,0x02);
					}
							
				}	else
				{	//不匹配
					APP_Response_SetWifiPassword(ID,0x01);
				}					
				break;
				
			case COMMAND_IOINITSTATUS:	//设置IO初始化状态	OK		
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_IOINITSTATUS,WIFI_RecvData);
				//先对比ECUID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ECUID12,12))
				{//匹配成功进行相应的操作
					//获取IO初始状态
					IO_Init_Status = WIFI_RecvData[23];
					APP_Response_IOInitStatus(ID,0x00);

					//保存新IO初始化状态到Flash
					//0：低电平（关闭心跳功能）1：高电平（打开心跳功能）
					Write_IO_INIT_STATU(&IO_Init_Status); 
					
				}else
				{
					APP_Response_IOInitStatus(ID,0x01);
				}
		}
	}
}


//串口事件处理
void process_UART1Event(void)
{
	char USART1_ECUID12[13] = {'\0'};
	char USART1_ECUID6[7] = {'\0'};
	char USART1_UID_NUM[2] = {0x00,0x00};
	char Channel[2] = "02";
	char ret =0;
	char testItem = 0;
	int AddNum = 0;
	UART1_ResolveFlag =  Resolve_Usart1RecvData((char *)USART1_RecvData,&UART1_Data_Len,&UART1_Command_Id);
	if(UART1_ResolveFlag == 0)
	{
		switch(UART1_Command_Id)
		{
			case COMMAND_SET_ID:				//设置ID
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SET_ID,USART1_RecvData);
				if(messageUsart1Len ==26)
				{
					memcpy(USART1_ECUID12,&USART1_RecvData[11],12);
					USART1_ECUID6[0] = ((USART1_ECUID12[0]-'0')<<4) + (USART1_ECUID12[1]-'0') ;
					USART1_ECUID6[1] = ((USART1_ECUID12[2]-'0')<<4) + (USART1_ECUID12[3]-'0') ;
					USART1_ECUID6[2] = ((USART1_ECUID12[3]-'0')<<4) + (USART1_ECUID12[5]-'0') ;
					USART1_ECUID6[3] = ((USART1_ECUID12[4]-'0')<<4) + (USART1_ECUID12[7]-'0') ;
					USART1_ECUID6[4] = ((USART1_ECUID12[5]-'0')<<4) + (USART1_ECUID12[9]-'0') ;
					USART1_ECUID6[5] = ((USART1_ECUID12[10]-'0')<<4) + (USART1_ECUID12[11]-'0') ;
					ret = Write_ECUID(USART1_ECUID6);													  		//ECU ID
					//设置WIFI密码
					if(ret != 0) 	USART1_Response_SET_ID(ret);
					Write_CHANNEL(Channel);
					
					IO_Init_Status= 1;
					Write_IO_INIT_STATU(&IO_Init_Status);
					//设置WIFI密码
					USART1_ECUID12[12] = '\0';
					delay_ms(5000);
					ret = WIFI_Factory(USART1_ECUID12);
					//写入WIFI密码
					Write_WIFI_PW("88888888",8);	//WIFI密码		
					init_ecu();
					USART1_Response_SET_ID(ret);
					init_ecu();	
				}else
				{
					USART1_Response_SET_ID(1);
				}

				break;
					
			case COMMAND_READ_ID:			//读取ID
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_READ_ID,USART1_RecvData);
				Read_ECUID(USART1_ECUID6);
				transformECUID(USART1_ECUID6,USART1_ECUID12);		//转换ECU ID
				USART1_ECUID12[12] = '\0';
				if(USART1_ECUID6[0] == 0xff)
				{
					USART1_Response_READ_ID(1,USART1_ECUID12);
				}else
				{
					USART1_Response_READ_ID(0,USART1_ECUID12);
				}
				
				break;
					
			case COMMAND_TEST:			//测试
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_TEST,USART1_RecvData);
				testItem =((USART1_RecvData[11]-'0')<<4) + (USART1_RecvData[12]-'0');
				ret = HardwareTest(testItem);
				SEGGER_RTT_printf(0, "%d %d\n",testItem,ret);
				USART1_Response_TEST(testItem,ret);
				break;
						
			case COMMAND_SET_NETWORK:	//组网
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_SET_NETWORK,USART1_RecvData);
				{	//匹配成功进行相应的操作
					
					SEGGER_RTT_printf(0, "COMMAND_SETNETWORK   Mapping\n");
							
					AddNum = (messageUsart1Len - 17)/6;
					//将数据写入EEPROM
					add_inverter(inverterInfo,AddNum,(char *)&USART1_RecvData[14]);
					USART1_Response_SET_NETWORK(0x00);
					init_inverter(inverterInfo);
					//进行一些绑定操作
					LED_off();
				}	
				break;
						
			case COMMAND_FACTORY:	//恢复到刚出厂的状态
				SEGGER_RTT_printf(0, "WIFI_Recv_Event%d %s\n",COMMAND_FACTORY,USART1_RecvData);
				ret = Write_UID_NUM(USART1_UID_NUM);
				USART1_Response_FACTORY(ret);
				init_inverter(inverterInfo);
			
				break;
		}
	}
}

//按键初始化密码事件处理
void process_KEYEvent(void)
{
	int ret =0,i = 0;
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event Start\n");
		delay_ms(5000);


	for(i = 0;i<3;i++)
	{
		
		ret = WIFI_Factory(ECUID12);
		if(ret == 0) break;
	}
	
	if(ret == 0) 	//写入WIFI密码
		Write_WIFI_PW("88888888",8);	//WIFI密码	
	
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event End\n");
}

//无线复位处理
void process_WIFI_RST(void)
{
	int ret =0,i = 0;
	SEGGER_RTT_printf(0, "process_WIFI_RST Start\n");
	for(i = 0;i<3;i++)
	{
		ret = WIFI_SoftReset();
		if(ret == 0) break;
	}
	
	SEGGER_RTT_printf(0, "process_WIFI_RST End\n");
}
