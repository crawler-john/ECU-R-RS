/*****************************************************************************/
/* File      : usart1_comm.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-04 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "USART1Comm.h"
#include "SEGGER_RTT.h"
#include "string.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//解析收到的数据
int Resolve_Usart1RecvData(char *RecvData,int* Data_Len,int* Command_Id)
{
	//APS
	if(strncmp(RecvData, "ECU", 3))
		return -1;
	//版本号 
	//长度
	*Data_Len = packetlen((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*10 + (RecvData[10]-'0');
	return 0;
}

void USART1_Response_SET_ID(char setflag)
{
	char SendData[100] = {'\0'};
	
	sprintf(SendData,"ECU11001601%02dEND",setflag);
	SEGGER_RTT_printf(0, "USART1_Response_SET_ID %s\n",SendData);
	UART1_SendData(SendData, 16);
}
void USART1_Response_READ_ID(char readflag,char *ECUID12)
{
	char SendData[100] = {'\0'};
	if(readflag == 0x00)
	{
		sprintf(SendData,"ECU11002802%02d%sEND",readflag,ECUID12);
	}else
	{
		sprintf(SendData,"ECU11002802%02d000000000000END",readflag);
	}
	
	SEGGER_RTT_printf(0, "USART1_Response_READ_ID %s\n",SendData);
	UART1_SendData(SendData, 28);
}
void USART1_Response_TEST(char testItem,char testflag)
{
	char SendData[100] = {'\0'};
	
	sprintf(SendData,"ECU11001803%02d%02dEND",testItem,testflag);
	SEGGER_RTT_printf(0, "USART1_Response_TEST %s\n",SendData);
	UART1_SendData(SendData, 18);
}
void USART1_Response_SET_NETWORK(char setflag)
{
	char SendData[100] = {'\0'};
	
	sprintf(SendData,"ECU11001604%02dEND",setflag);
	SEGGER_RTT_printf(0, "USART1_Response_SET_NETWORK %s\n",SendData);
	UART1_SendData(SendData, 16);
}
void USART1_Response_FACTORY(char setflag)
{
	char SendData[100] = {'\0'};
	
	sprintf(SendData,"ECU11001605%02dEND",setflag);
	SEGGER_RTT_printf(0, "USART1_Response_FACTORY %s\n",SendData);
	UART1_SendData(SendData, 16);
}
