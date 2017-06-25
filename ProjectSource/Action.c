/*
 * Action.c
 *
 *  Created on: 2017年4月3日
 *      Author: ZYF
 */



#include "stdType.h"
#include "DSP28x_Project.h"
#include "Timer.h"
#include "DeviceNet.h"
#include "RefParameter.h"
#include "SampleProcess.h"
#include "Action.h"

/**
 * 同步预制命令信息
 */
RefSynCommandMessage g_SynCommandMessage;



uint8_t  SendBufferDataAction[10];//接收缓冲数据
struct DefFrameData  ActionSendFrame; //接收帧处理


/**
 * 初始化使用的数据
 */
void ActionInit(void)
{

	ActionSendFrame.complteFlag = 0xff;
	ActionSendFrame.pBuffer = SendBufferDataAction;

	g_SynCommandMessage.synActionFlag = 0;
	g_SynCommandMessage.closeWaitAckTime.delayTime =  g_SystemLimit.syncReadyWaitTime;
	g_SynCommandMessage.closeWaitAckTime.startTime = 0;
	g_SynCommandMessage.closeWaitActionTime.delayTime =  g_SystemLimit.syncReadyWaitTime;
	g_SynCommandMessage.closeWaitActionTime.startTime = 0;
	g_SynCommandMessage.lastLen = 0;

}

static uint8_t SynCloseReadyAction(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);

/**
 * 引用帧服务
 *
 * @param  指向处理帧信息内容的指针
 * @param  指向发送帧信息的指针
 *
 * @retrun 错误代码
 * @bref   对完整帧进行提取判断
 */
uint8_t FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	uint8_t id = 0;
	uint8_t tempData[8] = {0};
	PointUint8 point;
	uint8_t result = 0;
	uint8_t codeStart = 0;
	uint8_t codeEnd = 0;
	ServiceDog();
	//最小长度必须大于0,且小于8对于单帧
	if ((pReciveFrame->len == 0) || (pReciveFrame->len > 8))
	{
		return ERROR_LEN ;
	}
	//接收帧ID必须大于等于0x30 --表示DSP控制指令
	id = pReciveFrame->pBuffer[0];
	if(id < 0x10)
	{
		return ERROR_UNIDENTIFIED_ID;
	}
	ServiceDog();
	switch (id)
	{

		case MasterParameterSetOne: //主站参数设置
		{
			ServiceDog();
			if (pReciveFrame->len >= 2) //ID+配置号+属性值 至少3字节
			{
				point.pData  = pReciveFrame->pBuffer + 2;
				point.len = pReciveFrame->len - 2;
				result = SetParamValue(pReciveFrame->pBuffer[1], &point);
				if(result)
				{
					return ERROR_SET_VALUE;
				}
				pSendFrame->pBuffer[0] = id| 0x80;
				pSendFrame->len = pReciveFrame->len;
				pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];//赋值功能码
				memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
				return 0;

			}
			break;
		}
		case MasterParameterRead:// 参数读取顺序结构
		{
			ServiceDog();
			if (pReciveFrame->len == 3) //ID+配置号1+配置号1  为3个字节
			{
				codeStart = pReciveFrame->pBuffer[1];
				codeEnd = pReciveFrame->pBuffer[2];
				if (codeEnd < codeStart) //结束值不小于开始值
				{
					return ERROR_INDEX;
				}

				for( ; codeStart <= codeEnd; codeStart++)
				{
					ServiceDog();
					point.pData  = tempData;
					point.len = 8;
					result = ReadParamValue(codeStart, &point); //一次只获取1个属性
					if(result)
					{
						//return 0xE3; //有中断则不能继续存储
						continue;//允许不连续的属性存在
					}
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = codeStart;//赋值功能码
					memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
					pSendFrame->len = point.len + 2;
					PacktIOMessage(pSendFrame);
				}
				pSendFrame->len = 0; //让底层禁止发送
				return 0;
			}
			break;
		}
		

		case ConfigMode:// 配置模式
		{
			ServiceDog();
			if (pReciveFrame->len == 4) //ID+配置号 至少2字节
			{
				if (pReciveFrame->pBuffer[1] != g_MacList[0])
				{
					return ERROR_LOCAL_MAC;
				}
				if (pReciveFrame->pBuffer[2] != DeviceNetObj.assign_info.master_MACID)
				{
					return ERROR_MASTER_MAC;
				}
				if (pReciveFrame->pBuffer[3] == EXIT_CONFIG)//离开配置模式
				{

					result = UpdateSystemSetData();
					g_WorkMode = pReciveFrame->pBuffer[3];
					 //应答回复
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = g_MacList[0];
					pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
					pSendFrame->pBuffer[3] = EXIT_CONFIG;
		 			pSendFrame->pBuffer[4] = result;
					pSendFrame->len = 5;
					return 0;

				}
				else if (pReciveFrame->pBuffer[3] == ENTER_CONFIG)//进入配置模式
				{
					g_WorkMode = pReciveFrame->pBuffer[3];
					//应答回复
					pSendFrame->pBuffer[0] = id| 0x80;
					memcpy(pSendFrame->pBuffer + 1, pReciveFrame->pBuffer + 1,
	 					 pReciveFrame->len - 1);
					pSendFrame->len = pReciveFrame->len;
					return 0;
				}

			}
			break;
		}
		case MutltiFrame://超过一帧数据读取结构
		{
			ServiceDog();
			if (pReciveFrame->len >= 2) //ID+配置号 至少2字节
			{
				ServiceDog();
				if (pReciveFrame->pBuffer[1] != MULTI_FRAME_FUNCTION)
				{
					return ERROR_FUNCTION;
				}
				SendMultiFrame(pSendFrame);

				pSendFrame->len = 0; //让底层禁止发送
				return 0;


			}


		}
		case SyncOrchestratorReadyClose: //同步合闸预制
		case SyncOrchestratorCloseAction: //同步合闸执行
		{
			return SynCloseReadyAction(pReciveFrame, pSendFrame);
		}

	}
	return 0xFF;



}
/**
 * 同步合闸动作--预制与执行
 *
 * @param  指向处理帧信息内容的指针
 * @param  指向发送帧信息的指针
 *
 * @retrun 错误代码
 * @bref   对完整帧进行提取判断
 */
static uint8_t SynCloseReadyAction(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	uint8_t id = 0;
	uint8_t count = 0;
	uint8_t i = 0;
	uint8_t ph[3] = {0};//三相选择
	uint16_t rad[3] = {0};//弧度归一化值
	float lastRatio = 0; //上一次比率
	uint8_t phase = 0;
	ServiceDog();

	//检测电压是否在范围内
	if(!CheckPhaseVoltageStatus(PHASE_A))
	{
		return ERROR_VOLTAGE;
	}


	id = pReciveFrame->pBuffer[0];
	switch (id)
	{
		case SyncOrchestratorReadyClose://同步合闸预制
		{
			ServiceDog();
			//必须不小于4
			if (pReciveFrame->len < 4)
			{
				return ERROR_LEN;
			}
			//必须为2的偶数倍
			if (pReciveFrame->len % 2 != 0)
			{
				return ERROR_LEN;
			}
			//计算容纳的路数
			count = (pReciveFrame->len - 2)/2;
			g_SynCommandMessage.loopByte = pReciveFrame->pBuffer[1];
			for(i = 0; i < count; i++)
			{
				ph[i] = (uint8_t)((g_SynCommandMessage.loopByte>>(2*i))&(0x03));
				rad[i] = pReciveFrame->pBuffer[2*i + 2] | ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
			}
			if (count == 3)
			{
				//相选择不能相同
				if ((ph[0] == ph[1]) || (ph[2] == ph[1]) || (ph[2] == ph[0]))
				{
					return ERROR_REPEAT_CHOICE;
				}

				//弧度必须依次增大
				if(!((rad[2] >= rad[1] )&&(rad[1] >= rad[0] )))
				{
					return ERROR_RAD_ASCEND;
				}

			}
			else if (count == 2)
			{
				if (ph[0] == ph[1])
				{
					return ERROR_REPEAT_CHOICE;

				}
				if(!(rad[1] >= rad[0] ))
				{
					return ERROR_RAD_ASCEND;
				}
			}

			//设置同步合闸参数
			 count = (pReciveFrame->len - 2)/2;
			 lastRatio = 0;
			 for(i = 0; i < count; i++)
			 {
				 phase= (pReciveFrame->pBuffer[1]>>(2*i))&(0x03);

				 //phase 位于1-3之间
				 if ((phase <= 3) && (phase >= 1))
				 {
					 g_PhaseActionRad[i].phase = phase - 1; //减1作为索引
				 }
				 else
				 {
					 return ERROR_PHASE_SCOPE;
				 }
				 g_PhaseActionRad[i].actionRad =  pReciveFrame->pBuffer[2*i + 2] + (((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8);


				 g_PhaseActionRad[i].realRatio =  (float)g_PhaseActionRad[i].actionRad / 65536   ;//累加计算绝对比率

				 //判断后一个相角不小于前一个
				 if (g_PhaseActionRad[i].realRatio < lastRatio)
				 {
					return ERROR_RAD_ASCEND;
				 }
				 g_PhaseActionRad[i].realDiffRatio =   g_PhaseActionRad[i].realRatio  - lastRatio;//相对上一级比率
				 lastRatio = g_PhaseActionRad[i].realRatio;

				 g_PhaseActionRad[i].count = count;//回路数量
			 }
			 //均是禁止，在同步执行状态下开启
			 for (i = count; i < 3; i++)
			 {
				 g_PhaseActionRad[i].enable = 0;
			 }

			memcpy(g_SynCommandMessage.commandData, pReciveFrame->pBuffer, pReciveFrame->len );//暂存指令
			g_SynCommandMessage.lastLen = pReciveFrame->len;
			g_SynCommandMessage.synActionFlag = SYN_HE_READY;//暂存指令标志
			g_SynCommandMessage.closeWaitAckTime.startTime = CpuTimer0.InterruptCount;
			//memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
			//pSendFrame->pBuffer[0] = id| 0x80;

			memcpy(ActionSendFrame.pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
			ActionSendFrame.pBuffer[0] = id| 0x80;
			ActionSendFrame.len = pReciveFrame->len;

			//pSendFrame->len = pReciveFrame->len;
			pSendFrame->len = 0;
			return 0;


		}
		case SyncOrchestratorCloseAction://同步合闸执行
		{
			ServiceDog();
			 //判断是否超时
			 if (!IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime, g_SynCommandMessage.closeWaitAckTime.delayTime))
			 {
				 if (g_SynCommandMessage.synActionFlag == SYN_HE_WAIT_ACTION)//是否已经预制
				 {
					 g_SynCommandMessage.synActionFlag = 0; //清空预制
					 if (pReciveFrame->len != g_SynCommandMessage.lastLen)
					 {
						 return ERROR_LEN;
					 }
					 //上一条指令是否为合闸预制
					 if (g_SynCommandMessage.commandData[i] != 0x30)
					 {
						 return ERROR_MATCHED_ID ;
					 }
					 //比较执行指令与预制指令是否相同
					 for(i = 1; i < pReciveFrame->len;i++)
					 {
						if (g_SynCommandMessage.commandData[i] != pReciveFrame->pBuffer[i])
						{
							return  ERROR_MATCHED_CMD;
						}
					 }
					 //设置同步合闸参数
					 count = (pReciveFrame->len - 2)/2;
					 for(i = 0; i < count; i++)
					 {
						 g_PhaseActionRad[i].enable = 0xFF;
					 }
					 //禁止合闸动作相角
					 for (i = count; i < 3; i++)
					 {
						 g_PhaseActionRad[i].enable = 0;
					 }

					 g_PhaseActionRad[0].loopByte = g_SynCommandMessage.commandData[1];
					 memcpy(ActionSendFrame.pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
					 ActionSendFrame.pBuffer[0] = id| 0x80;
					 ActionSendFrame.len = pReciveFrame->len;
					 g_SynCommandMessage.synActionFlag = SYN_HE_ACTION;//置同步合闸动作标志
					 g_SynCommandMessage.synActionFlag = SYN_HE_ACTION;//置同步合闸动作标志
					 pSendFrame->len = 0;//取消底层发送
					 return 0;

				 }
				 else
				 {
					 return ERROR_OPERATE_SEQUENCE;
				 }

			 }
			 else
			 {
				 g_SynCommandMessage.synActionFlag = 0;
				 for (i = count; i < 3; i++)
				 {
					 g_PhaseActionRad[i].enable = 0;
				 }
				 return ERROR_OVERTIME;
			 }
		}
	}
	return 0xFF;

}
/**
 * 同步执行应答
 *
 * @param  应答状态 0-正常，回复正常应答， 非0--一次代号
 *
 * @retrun null
 */
void SynActionAck(uint8_t state)
{
	ServiceDog();
	if (state != 0)
	{
		ActionSendFrame.pBuffer[0] = ErrorACK;
		ActionSendFrame.pBuffer[1] = SyncOrchestratorCloseAction;//同步执行
		ActionSendFrame.pBuffer[2] = state;
		ActionSendFrame.pBuffer[3] = 0xFF;
		ActionSendFrame.len = 4;
	}
	PacktIOMessage( &ActionSendFrame);
}


/**
 * 同步执行应答
 *
 * @param  应答状态 0-正常，回复正常应答， 非0--一次代号
 *
 * @retrun null
 */
void ErrorAck(uint8_t id,uint8_t state)
{
	ServiceDog();
	if (state != 0)
	{
		ActionSendFrame.pBuffer[0] = ErrorACK;
		ActionSendFrame.pBuffer[1] = id;//同步执行
		ActionSendFrame.pBuffer[2] = state;
		ActionSendFrame.pBuffer[3] = 0xFF;
		ActionSendFrame.len = 4;
	}

}
/**
 * 同步合闸等待应答状态
 * 判断是否为从站节点应答，判断是否是正常合闸预制应答，若是则置为相应的合闸预制状态，检测全部预制相是否齐全,
 * 如齐全设置时间等待预制时间T2。
 *
 * @param  pID  11bitID标识
 * @param  pbuff 缓冲数据
 * @param  len 数据长度
 *
 * @retrun 0--未进行处理，  非0--符合要求进行处理
 */
uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len)
{
	uint8_t i = 0, mac = 0;
	//是否为预制状态
	if ( g_SynCommandMessage.synActionFlag != SYN_HE_READY )
	{
		return 0;
	}
	ServiceDog();
	//是否超时
	if (IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime,
					g_SynCommandMessage.closeWaitAckTime.delayTime))
	{
		//复位
		g_SynCommandMessage.synActionFlag = 0;
		g_PhaseActionRad[0].readyFlag = 0;
		g_PhaseActionRad[1].readyFlag = 0;
		g_PhaseActionRad[2].readyFlag = 0;
		return 0;
	}
	if( ((*pID) & 0x03C0) != 0x03C0)  //GROUP1_POLL_STATUS_CYCLER_ACK
	{
		return 0;
	}
	ServiceDog();
	mac = GET_GROUP1_MAC(*pID);
	for( i = 0; i < g_PhaseActionRad[0].count; i++)
	{
		//地址是否来自A,B,C相
		if (g_MacList[g_PhaseActionRad[i].phase + 1] == mac)
		{
			if (len < 2)
			{
				return 0xff;
			}
			if (pbuff[0] == (SyncReadyClose | 0x80))//从站 同步合闸预制返回指令
			{
				g_PhaseActionRad[i].readyFlag = 0xff;

				//检测是否全部置为预制状态
				for(i = 0; i < g_PhaseActionRad[0].count; i++)
				{
					//有未准备相，返回
					if (g_PhaseActionRad[i].readyFlag == 0)
					{
						return 0xff;
					}
				}
				//全部项，已经就绪
				g_SynCommandMessage.synActionFlag = SYN_HE_WAIT_ACTION;
				g_SynCommandMessage.closeWaitActionTime.startTime = CpuTimer0.InterruptCount;
				PacktIOMessage( &ActionSendFrame);//发送应答
				return  0xff;
			}
		}
	}
	return 0xff;







}

/**
 * 发送帧数据
 *
 * @param  pSendFrame 指向发送帧信息的指针
 *
 * @retrun null
 */
void SendMultiFrame(struct DefFrameData* pSendFrame)
{
	uint8_t i = 0;
	uint8_t remain= SAMPLE_LEN % 3;
	uint8_t count = SAMPLE_LEN / 3;
	uint16_t temp = 0;
	uint8_t id  = 0x1B;

	for (i = 0; i < count ;i++)
	{
		ServiceDog();
		pSendFrame->pBuffer[0] = id| 0x80;
		pSendFrame->pBuffer[1] = i;
		temp =  (uint16_t)SampleDataSavefloat[3*i];
		pSendFrame->pBuffer[2] = (uint8_t)temp;
		pSendFrame->pBuffer[3] = (uint8_t)(temp>>8);
		temp =  (uint16_t)SampleDataSavefloat[3*i + 1];
		pSendFrame->pBuffer[4] = (uint8_t)temp;
		pSendFrame->pBuffer[5] = (uint8_t)(temp>>8);
		temp =  (uint16_t)SampleDataSavefloat[3*i + 2];
		pSendFrame->pBuffer[6] = (uint8_t)temp;
		pSendFrame->pBuffer[7] = (uint8_t)(temp>>8);

		if (remain ==0)
		{
			pSendFrame->pBuffer[1] = i |0x80;
		}
		pSendFrame->len = 8;
		PacktIOMessage(pSendFrame);
	}
	ServiceDog();
	if (remain !=0)
	{

		pSendFrame->pBuffer[0] = id| 0x80;
		pSendFrame->pBuffer[1] =  i |0x80;

		temp =  (uint16_t)SampleDataSavefloat[3*count];
		pSendFrame->pBuffer[2] = (uint8_t)temp;
		pSendFrame->pBuffer[3] = (uint8_t)(temp>>8);

		if (remain == 2)
		{
			temp =  (uint16_t)SampleDataSavefloat[3*i + 1];
			pSendFrame->pBuffer[4] = (uint8_t)temp;
			pSendFrame->pBuffer[5] = (uint8_t)(temp>>8);
		}
		pSendFrame->len = 2 + remain * 2;
		PacktIOMessage(pSendFrame);
	}


}




