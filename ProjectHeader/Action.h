/*
 * Action.h
 *
 *  Created on: 2017��4��3��
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_ACTION_H_
#define PROJECTHEADER_ACTION_H_
#include "DeviceNet.h"



extern void ActionDataInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
#endif /* PROJECTHEADER_ACTION_H_ */
