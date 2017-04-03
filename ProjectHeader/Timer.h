/*
 * Timer.h
 *
 *  Created on: 2017Äê4ÔÂ3ÈÕ
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_TIMER_H_
#define PROJECTHEADER_TIMER_H_

#include "stdType.h"

#ifdef	__cplusplus
extern "C" {
#endif



__interrupt void Cpu_timer0_isr(void);
uint8_t IsOverTime(uint32_t startTime, uint32_t delayTime);


#ifdef	__cplusplus
}
#endif

#endif /* PROJECTHEADER_TIMER_H_ */
