/*
 * Action.h
 *
 *  Created on: 2017Äê4ÔÂ3ÈÕ
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_ACTION_H_
#define PROJECTHEADER_ACTION_H_
#include "DeviceNet.h"



extern void ActionDataInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
#endif /* PROJECTHEADER_ACTION_H_ */
