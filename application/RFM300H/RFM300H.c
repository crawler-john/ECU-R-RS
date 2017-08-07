/*****************************************************************************/
/* File      : rfm300h.c                                                     */
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
#include "RFM300H.h"
#include "SEGGER_RTT.h"
#include "delay.h"
#include "string.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
//#define CMT2300_DEBUG

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int RFM300_Bind_Uid(char *ECUID,char *UID,char channel,char rate)
{
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[32] = {'\0'};
	char Recvdata[32] = {'\0'};

	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;
	Senddata[3]=0x08;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=channel;
	Senddata[17]=rate;
	Senddata[18]=0x00;
	Senddata[19]=0x00;
	Senddata[20]=0x00;
	Senddata[21]=0x00;
	Senddata[22]=0x00;
	Senddata[23]=0x00;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
		
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;
#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n",Senddata[i]);
#endif
	for(i=0;i<1;i++){
		SendMessage((unsigned char *)Senddata,28);
		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD8)&&		//表示绑定成功
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{
			SEGGER_RTT_printf(0, "RFM300_Bind_Uid %02x%02x%02x%02x%02x%02x\n",Senddata[10],Senddata[11],Senddata[12],Senddata[13],Senddata[14],Senddata[15]);
				return 1;
		}
		else
		{
			delay_ms(20);
			continue;
		}
			
	}
	return 0;
}

int RFM300_Heart_Beat(char *ECUID,char *UID,status_t* status,unsigned short *heart_rate,unsigned short * offNum,char *Ver)
{
	
	//char mos_Status = 0;
	char IO_InitStatus = 0;
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[32] = {'\0'};
	char Recvdata[32] = {'\0'};
	
	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=0x00;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=0x00;
	Senddata[17]=0x00;
	Senddata[18]=0x00;
	Senddata[19]=0x00;
	Senddata[20]=0x00;
	Senddata[21]=0x00;
	Senddata[22]=0x00;
	Senddata[23]=0x00;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;

#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif
	
	for(i=0;i<1;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD0)&&
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{

			//mos_Status = Recvdata[16];
			//SEGGER_RTT_printf(0,"Recvdata[16] : %d\n",mos_Status);
			//if(mos_Status == 0)
			//{	
				status->mos_status = 1;
			//}else
			//{
				//status->mos_status = 0;
			//}
			
			IO_InitStatus = Recvdata[17];
			if(IO_InitStatus == 0)
			{
				status->function_status = 1;
			}else
			{
				status->function_status = 0;
			}
			
			*offNum = Recvdata[18]*256+Recvdata[19];
			*heart_rate = Recvdata[20]*256 +Recvdata[21];
			*Ver =Recvdata[22];
			SEGGER_RTT_printf(0, "RFM300_Heart_Beat %02x%02x%02x%02x%02x%02x  %d %d %d %d %d \n",Senddata[10],Senddata[11],Senddata[12],Senddata[13],Senddata[14],Senddata[15],status->mos_status,status->function_status,*heart_rate,*offNum,*Ver);
			return 1;
		}
		else
		{
			delay_ms(20);
			continue;
		}
			
	}
	return 0;
}

int RFM300_IO_Init(char *ECUID,char *UID,char IO_Status,status_t* status,unsigned short *heart_rate,unsigned short * offNum,char *Ver)
{
	char mos_Status = 0;
	char IO_InitStatus = 0;

		
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[32] = {'\0'};
	char Recvdata[32] = {'\0'};
	
	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=IO_Status;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=0x00;
	Senddata[17]=0x00;
	Senddata[18]=0x00;
	Senddata[19]=0x00;
	Senddata[20]=0x00;
	Senddata[21]=0x00;
	Senddata[22]=0x00;
	Senddata[23]=0x00;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;
	
#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif
	
	for(i=0;i<3;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD1)&&
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{
			mos_Status = Recvdata[16];
			if(mos_Status == 0)
			{
				status->mos_status = 1;
			}else
			{
				status->mos_status = 0;
			}
			
			IO_InitStatus = Recvdata[17];
			if(IO_InitStatus == 0)
			{
				status->function_status = 1;
			}else
			{
				status->function_status = 0;
			}
			
			*offNum = Recvdata[18]*256+Recvdata[19];
			*heart_rate = Recvdata[20]*256 +Recvdata[21];
			*Ver =Recvdata[22];
			SEGGER_RTT_printf(0, "RFM300_IO_Init  %d %d %d %d %d \n",status->mos_status,status->function_status,*heart_rate,*offNum,*Ver);

			return 1;			
		}
		else
			continue;
	}
	return 0;
}

int RFM300_Set_Uid(char *ECUID,char *UID,int channel,int rate,char *NewUid,char *SaveChannel,char *SaveRate)
{
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[32] = {'\0'};
	char Recvdata[32] = {'\0'};

	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=0x09;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=channel;
	Senddata[17]=rate;
	Senddata[18]=NewUid[0];
	Senddata[19]=NewUid[1];
	Senddata[20]=NewUid[2];
	Senddata[21]=NewUid[3];
	Senddata[22]=NewUid[4];
	Senddata[23]=NewUid[5];
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;

	Senddata[26]=0xFE;
	Senddata[27]=0xFE;

#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n",Senddata[i]);
#endif
	
	for(i=0;i<3;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD9)&&
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[18]==Recvdata[10])&&
			(Senddata[19]==Recvdata[11])&&
			(Senddata[20]==Recvdata[12])&&
			(Senddata[21]==Recvdata[13])&&
			(Senddata[22]==Recvdata[14])&&
			(Senddata[23]==Recvdata[15]))		
		{
				*SaveChannel = Recvdata[16];
				*SaveRate = Recvdata[17];
				SEGGER_RTT_printf(0, "RFM300_Set_Uid  %d %d %d %d %d \n",*SaveChannel,*SaveRate);

				return 1;
		}
		else
				continue;
	}
	return 0;
}
