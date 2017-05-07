/*
 * SoftI2C.h
 *
 *  Created on: 2017Äê5ÔÂ7ÈÕ
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_SOFTI2C_H_
#define PROJECTHEADER_SOFTI2C_H_

#include "stdType.h"


void SoftI2CInit(void);
uint8_t I2C_IRcvB( uint8_t sla,  uint8_t suba, uint8_t *c);
uint8_t I2C_ISendB( uint8_t sla, uint8_t suba, uint8_t c);
uint8_t EEPROMWriteByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t data);
uint8_t EEPROMReadByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t* pData);





#endif /* PROJECTHEADER_SOFTI2C_H_ */
