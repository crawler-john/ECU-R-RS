/*****************************************************************************/
/* File      : delay.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#include "delay.h"
#include "sys.h"
 
/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//初始化延迟函数
void delay_init()	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/1000000;	//为系统时钟的1/8  

	fac_ms=(u16)fac_us*1000;//非ucos下,代表每个ms需要的systick时钟数   
}								    
//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对36M条件下,nms<=466 
static void _delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	  	    
} 


void delay_ms(u16 nms)
{
	int _nms = nms;
	if(_nms < 466)
	{
		_delay_ms(_nms);
	}else
	{
		while(_nms >=466)
		{
			_delay_ms(400);
			_nms-=400;
		}
		_delay_ms(_nms);
	}
}
































