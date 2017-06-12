/*
 * Action.h
 *
 *  Created on: 2017年4月3日
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_ACTION_H_
#define PROJECTHEADER_ACTION_H_
#include "DeviceNet.h"

typedef struct TagSynCommandMessage
{
	volatile uint8_t synActionFlag; //同步预制标志
	RunTimeStamp closeWaitAckTime;//同步预制等待应答时间戳
	RunTimeStamp closeWaitActionTime;//同步预制等待合闸时间戳
	uint8_t commandData[10]; //命令字
	uint8_t loopByte; //回路控制字
	uint8_t lastLen;//最新接收数据长度
}
RefSynCommandMessage;



#define SYN_HE_READY 0xAA     //预制状态--等待应答
#define SYN_HE_WAIT_ACTION 0xA5 //预制状态--等待执行命令
#define SYN_HE_ACTION 0x55

extern void ActionInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void  SendMultiFrame(struct DefFrameData* pSendFrame);
extern void SynActionAck(uint8_t state);
extern uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len);
//全局变量
extern RefSynCommandMessage g_SynCommandMessage;

#endif /* PROJECTHEADER_ACTION_H_ */
