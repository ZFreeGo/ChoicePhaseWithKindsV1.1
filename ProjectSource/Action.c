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


//暂存上一次命令字
uint8_t LastFlag = 0;
uint8_t CommandData[10] = {0};
uint8_t LastLen = 0;
uint32_t LastTime = 0;
uint8_t loopByte = 0;

/**
 * 初始化使用的数据
 */
void ActionDataInit(void)
{
	LastFlag = 0;
	LastTime = 0;
	LastLen = 0;
}


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


	uint8_t count = 0;
	uint8_t i = 0;

	uint8_t ph[3] = {0};//三相选择
	uint16_t rad[3] = {0};//弧度归一化值
	uint8_t tempData[8] = {0};
	PointUint8 point;
	uint8_t result = 0;
	uint8_t codeStart = 0;
	uint8_t codeEnd = 0;

	//uint8_t remain = 0;
	//uint16_t temp = 0;
	//最小长度必须大于0,且小于8对于单帧
	if ((pReciveFrame->len == 0) || (pReciveFrame->len > 8))
	{
		return 0XF1;
	}
	//接收帧ID必须大于等于0x30 --表示DSP控制指令
	id = pReciveFrame->pBuffer[0];
	if(id < 0x10)
	{
		return 0XF2;
	}

	switch (id)
	{

		case 0x11: //主站参数设置
		{
			if (pReciveFrame->len >= 2) //ID+配置号+属性值 至少3字节
			{
				point.pData  = pReciveFrame->pBuffer + 2;
				point.len = pReciveFrame->len - 2;
				result = SetParamValue(pReciveFrame->pBuffer[1], &point);
				if(result)
				{
					return 0xE1;
				}
				pSendFrame->pBuffer[0] = id| 0x80;
				pSendFrame->len = pReciveFrame->len;
				pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];//赋值功能码
				memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
				return 0;

			}
			break;
		}
		case 0x12:// 参数读取顺序结构
		{
			if (pReciveFrame->len == 3) //ID+配置号1+配置号1  为3个字节
			{
				codeStart = pReciveFrame->pBuffer[1];
				codeEnd = pReciveFrame->pBuffer[2];
				if (codeEnd < codeStart) //结束值不小于开始值
				{
					return 0xE2;
				}

				for( ; codeStart <= codeEnd; codeStart++)
				{
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
		
		case 0x13://参数读取非顺序结构
		{
			if (pReciveFrame->len >= 2) //ID+配置号 至少2字节
			{
				for( i = 1; i < pReciveFrame->len; i++)
				{
					point.pData  = tempData;
					point.len = 8;
					result = ReadParamValue(pReciveFrame->pBuffer[i], &point); //一次只获取1个属性
					if(result)
					{
						return 0xE4;
					}
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[i];//赋值功能码
					memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
					pSendFrame->len = point.len + 2;
					PacktIOMessage(pSendFrame);
				}
				pSendFrame->len = 0; //让底层禁止发送
				return 0;
			}

			break;
		}
		case 0x15:// 配置模式
		{
			if (pReciveFrame->len == 4) //ID+配置号 至少2字节
			{
				if (pReciveFrame->pBuffer[1] != g_LocalMac)
				{
					return 0xB0;
				}
				if (pReciveFrame->pBuffer[2] != DeviceNetObj.assign_info.master_MACID)
				{
					return 0xB1;
				}
				if (pReciveFrame->pBuffer[3] == 0x55)//离开配置模式
				{

					result = UpdateSystemSetData();
					g_WorkMode = pReciveFrame->pBuffer[3];
					 //应答回复
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = g_LocalMac;
					pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
					pSendFrame->pBuffer[3] = 0x55;
		 			pSendFrame->pBuffer[4] = result;
					pSendFrame->len = 5;
					return 0;

				}
				else if (pReciveFrame->pBuffer[3] == 0xAA)//进入配置模式
				{
					g_WorkMode = pReciveFrame->pBuffer[3];
					//应答回复
					pSendFrame->pBuffer[0] = id| 0x80;
					memcpy(pSendFrame->pBuffer + 1, pReciveFrame->pBuffer,
	 					 pReciveFrame->len - 1);
					pSendFrame->len = pReciveFrame->len;
					return 0;
				}

			}
			break;
		}
		case 0x1B://超过一帧数据读取结构
		{
			if (pReciveFrame->len >= 2) //ID+配置号 至少2字节
			{
				if (pReciveFrame->pBuffer[1] != 0xAA)
				{
					return 0xE5;
				}
				SendMultiFrame(pSendFrame);

				pSendFrame->len = 0; //让底层禁止发送
				return 0;


			}


		}


		case 0x30://同步合闸预制
		{
			//必须不小于4
			if (pReciveFrame->len < 4)
			{
				return 0XF3;
			}
			//必须为2的偶数倍
			if (pReciveFrame->len % 2 != 0)
			{
				return 0XF4;
			}
			//计算容纳的路数
			count = (pReciveFrame->len - 2)/2;
			loopByte = pReciveFrame->pBuffer[1];
			for(i = 0; i < count; i++)
			{
				ph[i] = (uint8_t)((loopByte>>(2*i))&(0x03));
				rad[i] = pReciveFrame->pBuffer[2*i + 2] | ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
			}
			if (count == 3)
			{
				//相选择不能相同
				if ((ph[0] == ph[1]) || (ph[2] == ph[1]) || (ph[2] == ph[0]))
				{
					return 0XF5;
				}

				//相数必须以此增大
				if(!((rad[2] >= rad[1] )&&(rad[1] >= rad[0] )))
				{
					return 0XF6;
				}

			}
			else if (count == 2)
			{
				if (ph[0] == ph[1])
				{
					return 0XF7;

				}
				if(!(rad[1] >= rad[0] ))
				{
					return 0XF8;
				}
			}

			memcpy(CommandData,pReciveFrame->pBuffer, pReciveFrame->len );//暂存指令
			LastLen = pReciveFrame->len;
			LastFlag = 0xAA;//暂存指令标志
			LastTime = CpuTimer0.InterruptCount;
			memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
			pSendFrame->pBuffer[0] = id| 0x80;
			pSendFrame->len = pReciveFrame->len;
			return 0;


		}
		case 0x31://同步合闸执行
		{
			 //判断是否超时
			 if (!IsOverTime(LastTime, g_SystemLimit.syncReadyWaitTime))
			 {
				 if (LastFlag == 0xAA)//是否已经预制
				 {
					 LastFlag = 0; //清空预制
					 if (pReciveFrame->len != LastLen)
					 {
						 return 0XF9;
					 }
					 //上一条指令是否为合闸预制
					 if (CommandData[i] != 0x30)
					 {
						 return 0XFA;
					 }
					 for(i = 1; i < pReciveFrame->len;i++)
					 {
						if (CommandData[i] != pReciveFrame->pBuffer[i])
						{
							return 0XFB;
						}
					 }
					 //设置同步合闸参数
					 count = (pReciveFrame->len - 2)/2;
					 for(i = 0; i < count; i++)
					 {
						 g_PhaseActionRad[i].phase = (CommandData[1]>>(2*i));
						 g_PhaseActionRad[i].actionRad =
								 pReciveFrame->pBuffer[2*i + 2] + ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
						 g_PhaseActionRad[i].enable = 0xFF;
						 g_PhaseActionRad[i].realRatio =   (float)g_PhaseActionRad[i].actionRad / 65536 ;
					 }
					 //禁止合闸动作相角
					 for (i = count; i < 3; i++)
					 {
						 g_PhaseActionRad[i].enable = 0;
					 }
					 g_PhaseActionRad[0].loopByte = CommandData[1];
					 memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
					 pSendFrame->pBuffer[0] = id| 0x80;
					 pSendFrame->len = pReciveFrame->len;
					 ZVDFlag = 0xFF; //设置同步预制命令
					 return 0;

				 }

			 }
			break;

		}
	}
	return 0xFF;



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
