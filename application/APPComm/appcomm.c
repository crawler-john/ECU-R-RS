/*****************************************************************************/
/* File      : appcomm.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-08 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "appcomm.h"
#include "string.h"
#include "SEGGER_RTT.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
static char SendData[MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9] = {'\0'};


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//解析收到的数据
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id)
{
	//APS
	if(strncmp(RecvData, "APS", 3))
		return -1;
	//版本号 
	//长度
	*Data_Len = packetlen((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*10 + (RecvData[10]-'0');
	return 0;
}

//ECU-RS获取基本信息回应
void APP_Response_BaseInfo(unsigned char *ID,char *ECU_NO,char *TYPE,char SIGNAL_LEVEL,char *SIGNAL_CHANNEL,int Length,char * Version)
{

	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);	
	sprintf(SendData,"a00000000APS11%04d01%s%03d%03d%s%03d%sEND\n",(37+Length),ECU_NO,101,SIGNAL_LEVEL,SIGNAL_CHANNEL,Length,Version);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];
	SEGGER_RTT_printf(0, "APP_Response_BaseInfo %s\n",&SendData[9]);
	WIFI_SendData(SendData, (47+Length));
}

//ECU-RS系统信息回应   mapflag   0表示匹配成功  1 表示匹配不成功
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int vaildNum)
{
	
	inverter_info *curinverter = inverter;
	unsigned char mos_a_function = 0;
	int i = 0;
	int length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);
	
	//SEGGER_RTT_printf(0, "SystemInfo %d %d  %d\n",mapflag,vaildNum,(MAXINVERTERCOUNT*INVERTERLENGTH + 16));

	if(mapflag == 1)
	{
		sprintf(SendData,"a00000000APS1100130201\n");
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];
		WIFI_SendData(SendData, 23);
		return;
	}else{
		
		sprintf(SendData,"a00000000APS1100130200");   //13字节
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];		
		
		for(i=0; (i<MAXINVERTERCOUNT)&&(i < vaildNum); i++)			
		{
			memcpy(&SendData[22+i*INVERTERLENGTH],curinverter->uid,6);
			SEGGER_RTT_printf(0, "SystemInfo %02x%02x%02x%02x%02x%02x   ",curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5]);
			
			SendData[28+i*INVERTERLENGTH] = curinverter->heart_rate /256;
			SendData[29+i*INVERTERLENGTH] = curinverter->heart_rate %256;

			SEGGER_RTT_printf(0, "heart_rate %d    ",curinverter->heart_rate);

			SendData[30+i*INVERTERLENGTH] = curinverter->off_times/256;
			SendData[31+i*INVERTERLENGTH] = curinverter->off_times%256;
			SEGGER_RTT_printf(0, "off_times %d    ",curinverter->off_times);

			mos_a_function = curinverter->status.mos_status;
			if(curinverter->status.function_status !=  0)	
			{
				mos_a_function |= (curinverter->status.function_status << 1) ;
			}
			SEGGER_RTT_printf(0, "mos:%d   function %d   ",curinverter->status.mos_status,curinverter->status.function_status);
			
			memcpy(&SendData[32+i*INVERTERLENGTH],&mos_a_function,1);
			memcpy(&SendData[33+i*INVERTERLENGTH],&curinverter->restartNum,1);
			
			SEGGER_RTT_printf(0, "mos_a_function %d\n",mos_a_function);
			
			
			curinverter++;
		}

		if(vaildNum > 0)
		{		
			length = (16+(vaildNum*INVERTERLENGTH));
			
			//改变报文字节长度
			SendData[14] = (length/1000) + '0';
			SendData[15] =	((length/100)%10) + '0';
			SendData[16] = ((length/10)%10) + '0';
			SendData[17] = ((length)%10) + '0';
			SendData[length-3+9] = 'E';
			SendData[length-2+9] = 'N';
			SendData[length-1+9] = 'D';
			SendData[length+9] = '\n';
		}else
		{
			length = 13;
			SendData[length+9] = '\n';
			
		}
		
#if 1
	for(i=0;i<length+9;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",SendData[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif	
		
		WIFI_SendData(SendData, (length+10));
	}


}

//ECU-RS设置组网回应
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);	
	sprintf(SendData,"a00000000APS11001303%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];	
	WIFI_SendData(SendData, 23);
}

//ECU-RS设置信道回应
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char *SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);
	if(mapflag == 1)
	{
		sprintf(SendData,"a00000000APS1100130401\n");
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];		
		WIFI_SendData(SendData, 23);
	}else{
		sprintf(SendData,"a00000000APS1100210400%s%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];		
		WIFI_SendData(SendData, 31);
	}
}

//ECU-RS设置WIFI密码
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);
	sprintf(SendData,"a00000000APS11001305%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];		
	WIFI_SendData(SendData, 23);
}


void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 16 + 9);
	sprintf(SendData,"a00000000APS11001306%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];		
	WIFI_SendData(SendData, 23);
}
