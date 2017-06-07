/*
 * Action.h
 *
 *  Created on: 2017年4月3日
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_ACTION_H_
#define PROJECTHEADER_ACTION_H_
#include "DeviceNet.h"

#define SYN_HE_READY 0xAA
#define SYN_HE_ACTION 0x55

extern void ActionInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void  SendMultiFrame(struct DefFrameData* pSendFrame);
extern void SynActionAck(uint8_t state);
//全局变量
extern volatile uint8_t g_SynAcctionFlag;
extern uint32_t g_ReadyHeLastTime;

#endif /* PROJECTHEADER_ACTION_H_ */
