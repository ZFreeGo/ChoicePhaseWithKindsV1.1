/*
 * Action.h
 *
 *  Created on: 2017��4��3��
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_ACTION_H_
#define PROJECTHEADER_ACTION_H_
#include "DeviceNet.h"

typedef struct TagSynCommandMessage
{
	volatile uint8_t synActionFlag; //ͬ��Ԥ�Ʊ�־
	RunTimeStamp closeWaitAckTime;//ͬ��Ԥ�Ƶȴ�Ӧ��ʱ���
	RunTimeStamp closeWaitActionTime;//ͬ��Ԥ�Ƶȴ���բʱ���
	uint8_t commandData[10]; //������
	uint8_t loopByte; //��·������
	uint8_t lastLen;//���½������ݳ���
}
RefSynCommandMessage;



#define SYN_HE_READY 0xAA     //Ԥ��״̬--�ȴ�Ӧ��
#define SYN_HE_WAIT_ACTION 0xA5 //Ԥ��״̬--�ȴ�ִ������
#define SYN_HE_ACTION 0x55

extern void ActionInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void  SendMultiFrame(struct DefFrameData* pSendFrame);
extern void SynActionAck(uint8_t state);
extern uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len);
//ȫ�ֱ���
extern RefSynCommandMessage g_SynCommandMessage;

#endif /* PROJECTHEADER_ACTION_H_ */
