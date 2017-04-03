/*
 * CAN.h
 *
 *  Created on: 2017Äê4ÔÂ1ÈÕ
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_CAN_H_
#define PROJECTHEADER_CAN_H_

#include "StdType.h"

#define GET_BIT0_7(data)   (uint8_t)(((uint32_t)(data)) & (0xFF));
#define GET_BIT8_15(data)  (uint8_t)(((uint32_t)(data)>>8) & (0xFF));
#define GET_BIT16_23(data) (uint8_t)(((uint32_t)(data)>>16) & (0xFF));
#define GET_BIT24_31(data) (uint8_t)(((uint32_t)(data)>>24) & (0xFF));


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

__interrupt void Can0Recive_ISR(void);
uint16_t InitStandardCAN(uint16_t id, uint16_t mask);
uint8_t CANSendData(uint16_t id, uint8_t * pbuff, uint8_t len);

#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif /* PROJECTHEADER_CAN_H_ */
