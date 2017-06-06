/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:PwmTimer.h
*文件标识:
*创建日期： 2015年1月29日
*摘要:
*上午11:03:47:创建本文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/



#ifndef PWMTIMER_H_
#define PWMTIMER_H_
#include "stdType.h"


#define TIME_BASE_2US 2 //时基2us
#define TIME_BASE_4US 4 //时基4us

#define CLKDIV_16 0b100 //16分频
#define CLKDIV_32 0b101 //32分频


// Configure which ePWM timer interrupts are enabled at the PIE level:
// 1 = enabled,  0 = disabled
#define PWM1_INT_ENABLE  0
#define PWM2_INT_ENABLE  1
#define PWM3_INT_ENABLE  1
#define PWM4_INT_ENABLE  1


// Configure the period for each timer
#define PWM1_TIMER_TBPRD   0x1FFF
#define PWM2_TIMER_TBPRD   0x1FFF
#define PWM3_TIMER_TBPRD   0x1FFF
#define PWM4_TIMER_TBPRD   0xFFFF


// Prototype statements for functions found within this file.
__interrupt void epwm1_timer_isr(void);
__interrupt void epwm2_timer_isr(void);
__interrupt void epwm3_timer_isr(void);
__interrupt void epwm4_timer_isr(void);

void InitEPwmTimer(void);
uint8_t  EPwm2TimerInit(uint32_t pulse, uint16_t base);
uint8_t  EPwm3TimerInit(uint32_t pulse, uint16_t base);
uint8_t  EPwm4TimerInit(uint32_t pulse, uint16_t base);
#endif /* PWMTIMER_H_ */
