/*****************************************************************************/
/* File      : delay.h                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif





























