/*****************************************************************************/
/* File      : key.h                                                         */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __KEY_H
#define __KEY_H	 

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define KEY_Reset  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
extern signed char KEY_FormatWIFI_Event;

void KEY_Init(void);//IO��ʼ��

void EXTIX_Init(void);//�ⲿ�жϳ�ʼ��


#endif