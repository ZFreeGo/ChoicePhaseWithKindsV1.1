/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:PwmTimer.c
*文件标识:
*创建日期： 2015年1月29日
*摘要:
*上午11:03:31:创建本文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "DSP28x_Project.h"
#include "PwmTimer.h"

Uint32  EPwm1TimerIntCount;
Uint32  EPwm2TimerIntCount;
Uint32  EPwm3TimerIntCount;
Uint32  EPwm4TimerIntCount;




void InitEPwmTimer()
{

   EPwm1TimerIntCount = 0;
   EPwm2TimerIntCount = 0;
   EPwm3TimerIntCount = 0;
   EPwm4TimerIntCount = 0;

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;

   // Setup Sync
   EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

  // EPwm1Regs.TBCTL.bit.PHSEN = TB_ENABLE;
  // EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
  // EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;

   EPwm1Regs.TBPHS.half.TBPHS = 100;
   EPwm2Regs.TBPHS.half.TBPHS = 200;
   EPwm3Regs.TBPHS.half.TBPHS = 300;
   EPwm4Regs.TBPHS.half.TBPHS = 400;

/*   EPwm1Regs.TBPRD = PWM1_TIMER_TBPRD;
   EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;    // Count up
   EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
   EPwm1Regs.ETSEL.bit.INTEN = PWM1_INT_ENABLE;  // Enable INT
   EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event


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
*/
   EPwm4Regs.TBPRD = PWM4_TIMER_TBPRD;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm4Regs.TBCTL.bit.CLKDIV = 0b110;  //64 分频  64*2
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = PWM4_INT_ENABLE;   // Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event

 //  EPwm4Regs.TBCTL.bit.FREE_SOFT = 2; //Free Run

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
   EDIS;

}
void EPwm4TimerInit(void)
{

   EPwm4TimerIntCount = 0;

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;

   // Setup Sync

   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

   EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;

   EPwm4Regs.TBPHS.half.TBPHS = 400;

   EPwm4Regs.TBPRD = PWM4_TIMER_TBPRD;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm4Regs.TBCTL.bit.CLKDIV = 0b110;  //64 分频  64*2
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = PWM4_INT_ENABLE;   // Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event


   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
   EDIS;

}
// Interrupt routines uses in this example:
__interrupt void epwm1_timer_isr(void)
{
   EPwm1TimerIntCount++;

   // Clear INT flag for this timer
   EPwm1Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm2_timer_isr(void)
{
   EPwm2TimerIntCount++;

   // Clear INT flag for this timer
   EPwm2Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm3_timer_isr(void)
{
   EPwm3TimerIntCount++;

   // Clear INT flag for this timer
   EPwm3Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//使用次定时器作为超时检测  2倍关系 换算方式 104ms
__interrupt void epwm4_timer_isr(void)
{
   //EPwm4TimerIntCount++;
   //if (EPwm4TimerIntCount >= 100) //约163.8ms
     //{
   EPwm4TimerIntCount = 0;
   EPwm4Regs.TBCTL.bit.CTRMODE = 3; //停止
   //ReciveOverTimeFlag = 0xffff;
    // }
   // Clear INT flag for this timer
   EPwm4Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

