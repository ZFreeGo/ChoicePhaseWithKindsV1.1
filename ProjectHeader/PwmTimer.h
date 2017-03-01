/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:PwmTimer.h
*�ļ���ʶ:
*�������ڣ� 2015��1��29��
*ժҪ:
*����11:03:47:�������ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/



#ifndef PWMTIMER_H_
#define PWMTIMER_H_

// Configure which ePWM timer interrupts are enabled at the PIE level:
// 1 = enabled,  0 = disabled
#define PWM1_INT_ENABLE  0
#define PWM2_INT_ENABLE  0
#define PWM3_INT_ENABLE  0
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
void EPwm4TimerInit(void);
#endif /* PWMTIMER_H_ */
