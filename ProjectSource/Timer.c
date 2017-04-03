/*
 * Timer.c
 *
 *  Created on: 2017年4月3日
 *      Author: ZYF
 */

#include "Timer.h"
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "DeviceIO.h"

/**
 * 判断时间是否超时
 *
 * @param   startTime 启动时间
 * @param   delayTime 延时时间
 *
 * @return  0xFF-时间到达 0-时间还未到达
 *
 * @bref   比较时间是否达到设定值，对溢出进行超时判断
 */
uint8_t IsOverTime(uint32_t startTime, uint32_t delayTime)

{
    if (UINT32_MAX - delayTime < startTime) //判断是否溢出,若溢出则进行则先判断是否超出一个周期
    {
         if( CpuTimer0.InterruptCount < startTime)//先判断是否小于startTime
         {
			if (CpuTimer0.InterruptCount >= (delayTime + startTime))
             {
                 return 0xFF;
             }
         }
    }
    else
    {
        if (CpuTimer0.InterruptCount >= startTime + delayTime)
        {
            return 0xFF;
        }
    }
    return 0;
}



__interrupt void Cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;
   TOGGLE_LED2;
   TOGGLE_LED1;
   TOGGLE_LED3;
   //if (FirstTrig)
   //{
	//   FirstTrig = 0;
	  // RESET_YONGCI_ACTION();
   //}
   // Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}



