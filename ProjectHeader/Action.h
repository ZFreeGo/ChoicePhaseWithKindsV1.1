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


	 SynTimeSequence = 0x34, //ʱ��ͬ�������ź�

    };


#define ERROR_LEN 					1
#define ERROR_UNIDENTIFIED_ID       2
#define ERROR_SET_VALUE      		3
#define ERROR_INDEX          		4
#define ERROR_LOCAL_MAC 			5
#define ERROR_MASTER_MAC 			6
#define ERROR_FUNCTION 				7
#define ERROR_VOLTAGE 				8
#define ERROR_REPEAT_CHOICE 		9
#define ERROR_RAD_ASCEND 			10
#define ERROR_PHASE_SCOPE 			11
#define ERROR_MATCHED_ID 			12
#define ERROR_MATCHED_CMD 		 	13
#define ERROR_OVERTIME 				14
#define ERROR_OPERATE_SEQUENCE 		15
#define ERROR_CAL_DELAY 			16
#define ERROR_ACTION_TIME 			17
#define ERROR_OVER_TOLERANCE 		18
#define ERROR_OUT_PULSE 			19
#define ERROR_COMPENSATION 			20
#define ERROR_REPEAT_READY          21
#define ERROR_SEQUENCE_UNRADY 		22
#define ERROR_SEQUENCE_CMD 			23
#define ERROR_SAVE_DATA      		24 //����Ĵ洢����EEPROM
#define ERROR_LOOP_COUNT     	25 //���������ģʽ
#define ERROR_SEQUENCE_MODE     	26 //����Ļ�·����
#define ERROR_WORK_MODE     	27 //����Ĺ���ģʽ

#define ENTER_CONFIG  0xAA
#define EXIT_CONFIG   0x55
#define MULTI_FRAME_FUNCTION 0xAA


#define SYN_HE_READY 0xAA     //Ԥ��״̬--�ȴ�Ӧ��
#define SYN_HE_WAIT_ACTION 0xA5 //Ԥ��״̬--�ȴ�ִ������
#define SYN_HE_ACTION 0x55
#define SYN_HE_SUCESS 0x1A

 /**
  *ʱ��ģʽ
  */
 #define TIME_SEQUENCE 0xA5



extern void ActionInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void  SendMultiFrame(struct DefFrameData* pSendFrame);
extern void SynActionAck(uint8_t state);
extern void ErrorAck(uint8_t id,uint8_t state);
extern uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len);
//ȫ�ֱ���
extern RefSynCommandMessage g_SynCommandMessage;



#endif /* PROJECTHEADER_ACTION_H_ */
