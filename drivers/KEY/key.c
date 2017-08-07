/*****************************************************************************/
/* File      : key.c                                                         */
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
#include "key.h"
#include <stm32f10x.h>
#include "SEGGER_RTT.h"
#include "delay.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/


int FirstStart = 1;
//����IO��ʼ��
void KEY_Init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��portB��ʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;//PB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��PB9
	
}

//�жϳ�ʼ��
void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  KEY_Init();	 //�����˿ڳ�ʼ��

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ø���ʱ�ӹ���
	
	//GPIOB.9 �ж����Լ��жϳ�ʼ������   �½��ش���
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource9);
 	EXTI_InitStructure.EXTI_Line=EXTI_Line9;	//KEY_RESET
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
 	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
 	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
 	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

 	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ���KEY_RESET���ڵ��ⲿ�ж�ͨ��
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//��ռ���ȼ�2
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
 	NVIC_Init(&NVIC_InitStructure);  

}
signed char KEY_FormatWIFI_Event = 0;
int keyflag = 0;
//�ⲿ�ж�9_5�������
void EXTI9_5_IRQHandler(void)
{
	delay_ms(20);
	if(FirstStart == 1)
	{
		FirstStart = 0;
		EXTI_ClearITPendingBit(EXTI_Line9); //���LINE9�ϵ��жϱ�־λz  
		return;
	}
	if(KEY_Reset==1)	 	 
	{
		keyflag++;
		if(keyflag > 1)
		{
			//�������ļ��ָ�����������
			//SEGGER_RTT_printf(0, "EXTI9_5_IRQHandler \n");
			keyflag = 0;
			KEY_FormatWIFI_Event = 1;
			
		}
		
	}
	EXTI_ClearITPendingBit(EXTI_Line9); //���LINE9�ϵ��жϱ�־λz  
}