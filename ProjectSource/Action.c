/*
 * Action.c
 *
 *  Created on: 2017��4��3��
 *      Author: ZYF
 */



#include "stdType.h"
#include "DSP28x_Project.h"
#include "Timer.h"
#include "DeviceNet.h"
#include "RefParameter.h"
#include "SampleProcess.h"


//�ݴ���һ��������
uint8_t LastFlag = 0;
uint8_t CommandData[10] = {0};
uint8_t LastLen = 0;
uint32_t LastTime = 0;


/**
 * ��ʼ��ʹ�õ�����
 */
void ActionDataInit(void)
{
	LastFlag = 0;
	LastTime = 0;
	LastLen = 0;
}


/**
 * ����֡����
 *
 * @param  ָ����֡��Ϣ���ݵ�ָ��
 * @param  ָ����֡��Ϣ��ָ��
 *
 * @retrun �������
 * @bref   ������֡������ȡ�ж�
 */
uint8_t FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	uint8_t id = 0;

	uint8_t loopByte = 0;
	uint8_t count = 0;
	uint8_t i = 0;

	uint8_t ph[3] = {0};//����ѡ��
	uint16_t rad[3] = {0};//���ȹ�һ��ֵ
	uint8_t tempData[8] = {0};
	PointUint8 point;
	uint8_t result = 0;
	//��С���ȱ������0,��С��8���ڵ�֡
	if ((pReciveFrame->len == 0) || (pReciveFrame->len > 8))
	{
		return 0XF1;
	}
	//����֡ID������ڵ���0x30 --��ʾDSP����ָ��
	id = pReciveFrame->pBuffer[0];
	if(id < 0x10)
	{
		return 0XF2;
	}

	switch (id)
	{

		case 0x11: //��վ��������
		{
			if (pReciveFrame->len >= 2) //ID+���ú�+����ֵ ����3�ֽ�
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
				pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];//��ֵ������
				memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
				return 0;

			}
			break;
		}
		case 0x13:
		{
			if (pReciveFrame->len >= 2) //ID+���ú� ����2�ֽ�
			{

				point.pData  = tempData;
				point.len = 8;
				result = ReadParamValue(pReciveFrame->pBuffer[1], &point); //һ��ֻ��ȡ1������
				if(result)
				{
					return 0xE2;
				}
				pSendFrame->pBuffer[0] = id| 0x80;
				pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];//��ֵ������
				memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
				pSendFrame->len = point.len + 2;
				return 0;

			}

			break;
		}
		case 0x30://ͬ����բԤ��
		case 0x32://ͬ����բԤ��
		{
			//���벻С��4
			if (pReciveFrame->len < 4)
			{
				return 0XF3;
			}
			//����Ϊ2��ż����
			if (pReciveFrame->len % 2 != 0)
			{
				return 0XF4;
			}
			//�������ɵ�·��
			count = (pReciveFrame->len - 2)/2;
			loopByte = pReciveFrame->pBuffer[1];
			for(i = 0; i < count; i++)
			{
				ph[i] = (uint8_t)((loopByte>>(2*i))&(0x03));
				rad[i] = pReciveFrame->pBuffer[2*i + 2] | ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
			}
			if (count == 3)
			{
				//��ѡ������ͬ
				if ((ph[0] == ph[1]) || (ph[2] == ph[1]) || (ph[2] == ph[0]))
				{
					return 0XF5;
				}

				//���������Դ�����
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

			memcpy(CommandData,pReciveFrame->pBuffer, pReciveFrame->len );//�ݴ�ָ��
			LastLen = pReciveFrame->len;
			LastFlag = 0xAA;//�ݴ�ָ���־
			LastTime = CpuTimer0.InterruptCount;
			memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
			pSendFrame->pBuffer[0] = id| 0x80;
			pSendFrame->len = pReciveFrame->len;
			return 0;


		}
		case 0x31://ͬ����բִ��
		{
			 //�ж��Ƿ�ʱ
			 if (!IsOverTime(LastTime, 5000))
			 {
				 if (LastFlag == 0xAA)//�Ƿ��Ѿ�Ԥ��
				 {
					 LastFlag = 0; //���Ԥ��
					 if (pReciveFrame->len != LastLen)
					 {
						 return 0XF9;
					 }
					 //��һ��ָ���Ƿ�Ϊ��բԤ��
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
					 //����ͬ����բ����
					 count = (pReciveFrame->len - 2)/2;
					 for(i = 0; i < count; i++)
					 {
						 g_PhaseActionRad[i].phase = (loopByte>>(2*i));
						 g_PhaseActionRad[i].actionRad =
								 pReciveFrame->pBuffer[2*i + 2] + ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
						 g_PhaseActionRad[i].enable = 0xFF;
					 }
					 //��ֹ��բ�������
					 for (i = count; i < 3; i++)
					 {
						 g_PhaseActionRad[i].enable = 0;
					 }
					 ZVDFlag = 0xFF;

					 memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
					 pSendFrame->pBuffer[0] = id| 0x80;
					 pSendFrame->len = pReciveFrame->len;
					 return 0;

				 }

			 }
			break;

		}
	}
	return 0xFF;



}
