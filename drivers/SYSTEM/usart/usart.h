/*****************************************************************************/
/* File      : usart.h                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

typedef enum
{ 
    EN_RECV_ST_GET_A         		= 0,		//接收数据头
    EN_RECV_ST_GET_ID          	= 1,	
    EN_RECV_ST_GET_HEAD         = 2,		//接收数据头
    EN_RECV_ST_GET_LEN          = 3,	//接收数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_DATA         = 4,	//接收数据部分数据
    EN_RECV_ST_GET_END          = 5		//接收END结尾标志
} eRecvSM;// receive state machin

#define USART1_REC_LEN			512

#define USART_REC_LEN  			1024  	//定义最大接收字节数 200
#define EN_USART2_RX 				1		//使能（1）/禁止（0）串口1接收
	  	

extern unsigned char WIFI_RecvData[USART_REC_LEN];
extern unsigned char WIFI_Recv_Event;
extern unsigned char USART1_RecvData[USART1_REC_LEN];
extern unsigned char USART1_Recv_Event;

unsigned short packetlen(unsigned char *packet);
int UART1_SendData(char *data, int num);
int WIFI_SendData(char *data, int num);
void WIFI_GetEvent(int *messageLen,unsigned char *ID);
void USART1_GetEvent(int *messageLen);
void uart2_init(u32 bound);
void uart1_init(u32 bound);

int AT(void);
int AT_ENTM(void);
int AT_WAP(char *ECUID12);
int AT_WAKEY(char *NewPasswd);
int WIFI_ChangePasswd(char *NewPasswd);
int WIFI_Reset(void);
int AT_Z(void);
int WIFI_ClearPasswd(void);
int WIFI_SoftReset(void);

int WIFI_Test(void);
int WIFI_Factory(char *ECUID12);
#endif


