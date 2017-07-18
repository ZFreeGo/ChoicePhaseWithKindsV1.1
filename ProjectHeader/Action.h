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


 enum CommandIdentify
{
	/// <summary>
	/// 合闸预制
	/// </summary>
	ReadyClose = 1,
	/// <summary>
	/// 合闸执行
	/// </summary>
	CloseAction = 2,
	/// <summary>
	/// 分闸预制
	/// </summary>
	ReadyOpen = 3,
	/// <summary>
	/// 分闸执行
	/// </summary>
	OpenAction = 4,

	/// <summary>
	/// 同步合闸预制
	/// </summary>
	SyncReadyClose = 5,
	/// <summary>
	/// 同步分闸预制
	/// </summary>
	//SyncReadyClose = 6,





	/// <summary>
	/// 主站参数设置,顺序
	/// </summary>
  //  MasterParameterSet = 0x10,
	// <summary>
	/// 主站参数设置，非顺序——按点设置
	/// </summary>
	MasterParameterSetOne = 0x11,
	/// <summary>
	/// 主站参数读取，顺序
	/// </summary>
	MasterParameterRead = 0x12,
	/// <summary>
	/// 主站参数读取，非顺序——按点读取
	/// </summary>
   // MasterParameterReadOne = 0x13,

	/// <summary>
	/// 错误
	/// </summary>
	ErrorACK = 0x14,


	/// <summary>
	/// 配置模式
	/// </summary>
	ConfigMode = 0x15,

	/// <summary>
	/// 子站状态改变信息上传
	/// </summary>
	SubstationStatuesChange = 0x1A,

	/// <summary>
	/// 多帧数据
	/// </summary>
	MutltiFrame = 0x1B,

	/// <summary>
	/// 同步控制器 合闸预制
	/// </summary>
	SyncOrchestratorReadyClose = 0x30,
	/// <summary>
	/// 同步控制器 合闸执行
	/// </summary>
	SyncOrchestratorCloseAction = 0x31,
   // SyncOrchestratorReadyOpen = 0x32,
	//SyncOrchestratorOpenAction = 0x32,


	 SynTimeSequence = 0x34, //时序同步脉冲信号

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
#define ERROR_SAVE_DATA      		24 //错误的存储数据EEPROM
#define ERROR_LOOP_COUNT     	25 //错误的序列模式
#define ERROR_SEQUENCE_MODE     	26 //错误的回路数量
#define ERROR_WORK_MODE     	27 //错误的工作模式

#define ENTER_CONFIG  0xAA
#define EXIT_CONFIG   0x55
#define MULTI_FRAME_FUNCTION 0xAA


#define SYN_HE_READY 0xAA     //预制状态--等待应答
#define SYN_HE_WAIT_ACTION 0xA5 //预制状态--等待执行命令
#define SYN_HE_ACTION 0x55
#define SYN_HE_SUCESS 0x1A

 /**
  *时序模式
  */
 #define TIME_SEQUENCE 0xA5



extern void ActionInit(void);
extern uint8_t  FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void  SendMultiFrame(struct DefFrameData* pSendFrame);
extern void SynActionAck(uint8_t state);
extern void ErrorAck(uint8_t id,uint8_t state);
extern uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len);
//全局变量
extern RefSynCommandMessage g_SynCommandMessage;



#endif /* PROJECTHEADER_ACTION_H_ */
