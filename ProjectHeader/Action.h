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


 enum CommandIdentify
{
	/// <summary>
	/// ��բԤ��
	/// </summary>
	ReadyClose = 1,
	/// <summary>
	/// ��բִ��
	/// </summary>
	CloseAction = 2,
	/// <summary>
	/// ��բԤ��
	/// </summary>
	ReadyOpen = 3,
	/// <summary>
	/// ��բִ��
	/// </summary>
	OpenAction = 4,

	/// <summary>
	/// ͬ����բԤ��
	/// </summary>
	SyncReadyClose = 5,
	/// <summary>
	/// ͬ����բԤ��
	/// </summary>
	//SyncReadyClose = 6,





	/// <summary>
	/// ��վ��������,˳��
	/// </summary>
  //  MasterParameterSet = 0x10,
	// <summary>
	/// ��վ�������ã���˳�򡪡���������
	/// </summary>
	MasterParameterSetOne = 0x11,
	/// <summary>
	/// ��վ������ȡ��˳��
	/// </summary>
	MasterParameterRead = 0x12,
	/// <summary>
	/// ��վ������ȡ����˳�򡪡������ȡ
	/// </summary>
   // MasterParameterReadOne = 0x13,

	/// <summary>
	/// ����
	/// </summary>
	ErrorACK = 0x14,


	/// <summary>
	/// ����ģʽ
	/// </summary>
	ConfigMode = 0x15,

	/// <summary>
	/// ��վ״̬�ı���Ϣ�ϴ�
	/// </summary>
	SubstationStatuesChange = 0x1A,

	/// <summary>
	/// ��֡����
	/// </summary>
	MutltiFrame = 0x1B,

	/// <summary>
	/// ͬ�������� ��բԤ��
	/// </summary>
	SyncOrchestratorReadyClose = 0x30,
	/// <summary>
	/// ͬ�������� ��բִ��
	/// </summary>
	SyncOrchestratorCloseAction = 0x31,
   // SyncOrchestratorReadyOpen = 0x32,
	//SyncOrchestratorOpenAction = 0x32,




    };


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
