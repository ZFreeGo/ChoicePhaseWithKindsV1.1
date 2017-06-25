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




    };


#define ERROR_LEN 			 0x11
#define ERROR_UNIDENTIFIED_ID      0x12
#define ERROR_SET_VALUE      0x13
#define ERROR_INDEX          0x14
#define ERROR_LOCAL_MAC 0x15
#define ERROR_MASTER_MAC 0x16
#define ERROR_FUNCTION 0x17
#define ERROR_VOLTAGE 0x18
#define ERROR_REPEAT_CHOICE 0x19
#define ERROR_RAD_ASCEND 0x20
#define ERROR_PHASE_SCOPE 0x21
#define ERROR_MATCHED_ID 0x22
#define ERROR_MATCHED_CMD 0x23
#define ERROR_OVERTIME 0x24
#define ERROR_OPERATE_SEQUENCE 0x25


#define ENTER_CONFIG  0xAA
#define EXIT_CONFIG   0x55
#define MULTI_FRAME_FUNCTION 0xAA


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
