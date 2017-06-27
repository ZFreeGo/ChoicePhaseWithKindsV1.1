/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:PwmTimer.c
*�ļ���ʶ:
*�������ڣ� 2015��1��29��
*ժҪ:
*����11:03:31:�������ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#include "DSP28x_Project.h"
#include "PwmTimer.h"
#include "DeviceIO.h"
#include "refParameter.h"

uint32_t  EPwm2TimerIntCount;
uint32_t  EPwm3TimerIntCount;
uint32_t  EPwm4TimerIntCount;


uint32_t  EPwm2TimerPeriod;
uint32_t  EPwm3TimerPeriod;
/**
 *  EPwm4Timer����
 */
uint32_t  EPwm4TimerPeriod;

void InitEPwmTimer(void)
{


   EPwm2TimerIntCount = 0;
   EPwm3TimerIntCount = 0;
   EPwm4TimerIntCount = 0;

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;

   // Setup Sync

   EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed


   //��ֹ��λ������TBPHS.half.TBPHSֵ
   EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
   EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
   EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;


   EPwm2Regs.TBPHS.half.TBPHS = 200;
   EPwm3Regs.TBPHS.half.TBPHS = 300;
   EPwm4Regs.TBPHS.half.TBPHS = 400;




   EPwm2Regs.TBPRD = PWM2_TIMER_TBPRD;
   EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm2Regs.ETSEL.bit.INTEN = PWM2_INT_ENABLE;   // Enable INT
   EPwm2Regs.ETPS.bit.INTPRD = ET_2ND;            // Generate INT on 2nd event


   EPwm3Regs.TBPRD = PWM3_TIMER_TBPRD;
   EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm3Regs.ETSEL.bit.INTEN = PWM3_INT_ENABLE;   // Enable INT
   EPwm3Regs.ETPS.bit.INTPRD = ET_3RD;            // Generate INT on 3rd event



   EPwm4Regs.TBPRD = PWM4_TIMER_TBPRD;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up

   //65536*1/80*10*32= 65536*4= 262 144 us(���ʱ����)
   EPwm4Regs.TBCTL.bit.CLKDIV = 0b101;  //32��Ƶ TBCLK = SYSCLKOUT / (HSPCLKDIV �� CLKDIV)
   EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0b101;// /10��Ƶ 10
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = PWM4_INT_ENABLE;   // Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event



 //  EPwm4Regs.TBCTL.bit.FREE_SOFT = 2; //Free Run

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
   EDIS;

}

/**
 * PwmTimer4 ��ʼ��
 *
 * @param pulse ��ʱ����
 * @param base  ʱ��TIME_BASE_2US/TIME_BASE_4US
 *
 * @return ������������Ϊcount����������ʾȫ�����
 * @brief �����ֵ
 */
uint8_t  EPwm4TimerInit(uint32_t pulse, uint16_t base)
{
	uint16_t div = 0;

   if ( base == TIME_BASE_2US) //TimeBase  1/80*10*16=2us
   {
	   div = CLKDIV_16;
	   EPwm4TimerPeriod = pulse >> 1;
   }
   else  if ( base  == TIME_BASE_4US) //TimeBase  1/80*10*32=4us
   {
	   div = CLKDIV_32;
	   EPwm4TimerPeriod = pulse >> 2;
   }
   else
   {
	   return 0xFF;
   }

   EPwm4TimerIntCount = 0;
   EPwm4Regs.TBCTL.bit.CTRMODE = 3;//ֹͣ������

   EALLOW;
  // SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;


   // Setup Sync

   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

   EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;
   EPwm4Regs.TBPHS.half.TBPHS = 400;



   //65536*1/80*10*32= 65536*4= 262 144 us(���ʱ����)
   EPwm4Regs.TBCTL.bit.CLKDIV = div;  //32��Ƶ TBCLK = SYSCLKOUT / (HSPCLKDIV �� CLKDIV)
   EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0b101;// /10��Ƶ 10
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = PWM4_INT_ENABLE;   // Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event

   EPwm4Regs.TBPRD = EPwm4TimerPeriod;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm4Regs.TBCTR = 0;//��ռ���

   EALLOW;
   if (SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC == 0)
   {
	   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//Start all the timers synced
   }
   EDIS;
   return 0;

}
/**
 * PwmTimer3 ��ʼ��
 *
 * @param pulse ��ʱ����
 * @param base  ʱ��TIME_BASE_2US/TIME_BASE_4US
 *
 * @return ������������Ϊcount����������ʾȫ�����
 * @brief �����ֵ
 */
uint8_t  EPwm3TimerInit(uint32_t pulse, uint16_t base)
{
	uint16_t div = 0;

   if ( base == TIME_BASE_2US) //TimeBase  1/80*10*16=2us
   {
	   div = CLKDIV_16;
	   EPwm3TimerPeriod = pulse >> 1;
   }
   else  if ( base  == TIME_BASE_4US) //TimeBase  1/80*10*32=4us
   {
	   div = CLKDIV_32;
	   EPwm3TimerPeriod = pulse >> 2;
   }
   else
   {
	   return 0xFF;
   }
   EPwm3TimerIntCount = 0;

   EPwm3Regs.TBCTL.bit.CTRMODE = 3;//ֹͣ������

   EALLOW;
  // SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;


   // Setup Sync

   EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

   EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
   EPwm3Regs.TBPHS.half.TBPHS = 400;



   //65536*1/80*10*32= 65536*4= 262 144 us(���ʱ����)
   EPwm3Regs.TBCTL.bit.CLKDIV = div;  //32��Ƶ TBCLK = SYSCLKOUT / (HSPCLKDIV �� CLKDIV)
   EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0b101;// /10��Ƶ 10
   EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm3Regs.ETSEL.bit.INTEN = PWM3_INT_ENABLE;   // Enable INT
   EPwm3Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event

   EPwm3Regs.TBPRD = EPwm3TimerPeriod;
   EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm3Regs.TBCTR = 0;//��ռ���

   EALLOW;
   if (SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC == 0)
   {
	   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//Start all the timers synced
   }
   EDIS;
   return 0;

}
/**
 * PwmTimer2 ��ʼ��
 *
 * @param pulse ��ʱ����
 * @param base  ʱ��TIME_BASE_2US/TIME_BASE_4US
 *
 * @return ������������Ϊcount����������ʾȫ�����
 * @brief �����ֵ
 */
uint8_t  EPwm2TimerInit(uint32_t pulse, uint16_t base)
{
	uint16_t div = 0;

   if ( base == TIME_BASE_2US) //TimeBase  1/80*10*16=2us
   {
	   div = CLKDIV_16;
	   EPwm2TimerPeriod = pulse >> 1;
   }
   else  if ( base  == TIME_BASE_4US) //TimeBase  1/80*10*32=4us
   {
	   div = CLKDIV_32;
	   EPwm2TimerPeriod = pulse >> 2;
   }
   else
   {
	   return 0xFF;
   }

   EPwm2TimerIntCount = 0;
   EPwm2Regs.TBCTL.bit.CTRMODE = 3;//ֹͣ������

   EALLOW;
  // SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;


   // Setup Sync

   EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

   EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
   EPwm2Regs.TBPHS.half.TBPHS = 400;



   //65536*1/80*10*32= 65536*4= 262 144 us(���ʱ����)
   EPwm2Regs.TBCTL.bit.CLKDIV = div;  //32��Ƶ TBCLK = SYSCLKOUT / (HSPCLKDIV �� CLKDIV)
   EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0b101;// /10��Ƶ 10
   EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm2Regs.ETSEL.bit.INTEN = PWM2_INT_ENABLE;   // Enable INT
   EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event

   EPwm2Regs.TBPRD = EPwm2TimerPeriod;
   EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm2Regs.TBCTR = 0;//��ռ���

   EALLOW;
   if (SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC == 0)
   {
	   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//Start all the timers synced
   }
   EDIS;
   return 0;

}
// Interrupt routines uses in this example:
__interrupt void epwm1_timer_isr(void)
{
   //EPwm1TimerIntCount++;

   // Clear INT flag for this timer
   EPwm1Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm2_timer_isr(void)
{
	if (EPwm2TimerIntCount == 0) //�״ν�����������ʱ��Ϊ100us
	{
		EPwm2TimerInit(100, TIME_BASE_2US);

	}
	else
	{
#ifdef INTEG_MODE
		if (EPwm2TimerIntCount % 2 == 1)
		{
			SET_OUTB1_H;
		}
		else
		{
			SET_OUTB1_L;

		}
		if (EPwm2TimerIntCount >= PULSE_COUNT)
		{
			EPwm2Regs.TBCTL.bit.CTRMODE = 3; //ֹͣ
			SET_OUTB1_L;
		}
#else
		if (EPwm2TimerIntCount % 2 == 1)
		{
		    SET_OUTA1_H;
		}
		else
		{
		    SET_OUTA1_L
		}
		if (EPwm2TimerIntCount >=  PULSE_COUNT)
		{
			EPwm2Regs.TBCTL.bit.CTRMODE = 3; //ֹͣ
		    SET_OUTA1_L;
		}
#endif
	}

	EPwm2TimerIntCount++;
   // Clear INT flag for this timer
   EPwm2Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm3_timer_isr(void)
{

	if (EPwm3TimerIntCount == 0) //�״ν�����������ʱ��Ϊ100us
	{
		EPwm3TimerInit(100, TIME_BASE_2US);

	}
	else
	{
		if (EPwm3TimerIntCount % 2 == 1)
		{
			SET_OUTB3_H;
		}
		else
		{
			SET_OUTB3_L;
		}
		if (EPwm3TimerIntCount >=  PULSE_COUNT)
		{
			EPwm3Regs.TBCTL.bit.CTRMODE = 3; //ֹͣ
			SET_OUTB3_L;
		}
	}
	EPwm3TimerIntCount++;

   // Clear INT flag for this timer
   EPwm3Regs.ETCLR.bit.INT = 1;
   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm4_timer_isr(void) //40M
{
	if (EPwm4TimerIntCount == 0) //�״ν�����������ʱ��Ϊ100us
	{
		EPwm4TimerInit(100, TIME_BASE_2US);

	}
	else
	{
		if (EPwm4TimerIntCount % 2 == 1)
		{
			SET_OUTB4_H;
		}
		else
		{
			SET_OUTB4_L;
		}
		if (EPwm4TimerIntCount >=  PULSE_COUNT)
		{
			EPwm2Regs.TBCTL.bit.CTRMODE = 3; //ֹͣ
			SET_OUTB4_L;
		}
	}
	EPwm4TimerIntCount++;

   // Clear INT flag for this timer
   EPwm4Regs.ETCLR.bit.INT = 1;
   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

