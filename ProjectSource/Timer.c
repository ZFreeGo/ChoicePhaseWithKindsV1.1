/*
 * Timer.c
 *
 *  Created on: 2017��4��3��
 *      Author: ZYF
 */

#include "Timer.h"
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "DeviceIO.h"

/**
 * �ж�ʱ���Ƿ�ʱ
 *
 * @param   startTime ����ʱ��
 * @param   delayTime ��ʱʱ��
 *
 * @return  0xFF-ʱ�䵽�� 0-ʱ�仹δ����
 *
 * @bref   �Ƚ�ʱ���Ƿ�ﵽ�趨ֵ����������г�ʱ�ж�
 */
uint8_t IsOverTime(uint32_t startTime, uint32_t delayTime)

{
    if (UINT32_MAX - delayTime < startTime) //�ж��Ƿ����,���������������ж��Ƿ񳬳�һ������
    {
         if( CpuTimer0.InterruptCount < startTime)//���ж��Ƿ�С��startTime
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



