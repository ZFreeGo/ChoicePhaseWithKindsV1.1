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
uint8_t loopByte = 0;

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


	uint8_t count = 0;
	uint8_t i = 0;

	uint8_t ph[3] = {0};//����ѡ��
	uint16_t rad[3] = {0};//���ȹ�һ��ֵ
	uint8_t tempData[8] = {0};
	PointUint8 point;
	uint8_t result = 0;
	uint8_t codeStart = 0;
	uint8_t codeEnd = 0;

	//uint8_t remain = 0;
	//uint16_t temp = 0;
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
		case 0x12:// ������ȡ˳��ṹ
		{
			if (pReciveFrame->len == 3) //ID+���ú�1+���ú�1  Ϊ3���ֽ�
			{
				codeStart = pReciveFrame->pBuffer[1];
				codeEnd = pReciveFrame->pBuffer[2];
				if (codeEnd < codeStart) //����ֵ��С�ڿ�ʼֵ
				{
					return 0xE2;
				}

				for( ; codeStart <= codeEnd; codeStart++)
				{
					point.pData  = tempData;
					point.len = 8;
					result = ReadParamValue(codeStart, &point); //һ��ֻ��ȡ1������
					if(result)
					{
						//return 0xE3; //���ж����ܼ����洢
						continue;//�������������Դ���
					}
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = codeStart;//��ֵ������
					memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
					pSendFrame->len = point.len + 2;
					PacktIOMessage(pSendFrame);
				}
				pSendFrame->len = 0; //�õײ��ֹ����
				return 0;
			}
			break;
		}
		
		case 0x13://������ȡ��˳��ṹ
		{
			if (pReciveFrame->len >= 2) //ID+���ú� ����2�ֽ�
			{
				for( i = 1; i < pReciveFrame->len; i++)
				{
					point.pData  = tempData;
					point.len = 8;
					result = ReadParamValue(pReciveFrame->pBuffer[i], &point); //һ��ֻ��ȡ1������
					if(result)
					{
						return 0xE4;
					}
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[i];//��ֵ������
					memcpy(pSendFrame->pBuffer + 2, point.pData, point.len );
					pSendFrame->len = point.len + 2;
					PacktIOMessage(pSendFrame);
				}
				pSendFrame->len = 0; //�õײ��ֹ����
				return 0;
			}

			break;
		}
		case 0x15:// ����ģʽ
		{
			if (pReciveFrame->len == 4) //ID+���ú� ����2�ֽ�
			{
				if (pReciveFrame->pBuffer[1] != g_LocalMac)
				{
					return 0xB0;
				}
				if (pReciveFrame->pBuffer[2] != DeviceNetObj.assign_info.master_MACID)
				{
					return 0xB1;
				}
				if (pReciveFrame->pBuffer[3] == 0x55)//�뿪����ģʽ
				{

					result = UpdateSystemSetData();
					g_WorkMode = pReciveFrame->pBuffer[3];
					 //Ӧ��ظ�
					pSendFrame->pBuffer[0] = id| 0x80;
					pSendFrame->pBuffer[1] = g_LocalMac;
					pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
					pSendFrame->pBuffer[3] = 0x55;
		 			pSendFrame->pBuffer[4] = result;
					pSendFrame->len = 5;
					return 0;

				}
				else if (pReciveFrame->pBuffer[3] == 0xAA)//��������ģʽ
				{
					g_WorkMode = pReciveFrame->pBuffer[3];
					//Ӧ��ظ�
					pSendFrame->pBuffer[0] = id| 0x80;
					memcpy(pSendFrame->pBuffer + 1, pReciveFrame->pBuffer,
	 					 pReciveFrame->len - 1);
					pSendFrame->len = pReciveFrame->len;
					return 0;
				}

			}
			break;
		}
		case 0x1B://����һ֡���ݶ�ȡ�ṹ
		{
			if (pReciveFrame->len >= 2) //ID+���ú� ����2�ֽ�
			{
				if (pReciveFrame->pBuffer[1] != 0xAA)
				{
					return 0xE5;
				}
				SendMultiFrame(pSendFrame);

				pSendFrame->len = 0; //�õײ��ֹ����
				return 0;


			}


		}


		case 0x30://ͬ����բԤ��
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
			 if (!IsOverTime(LastTime, g_SystemLimit.syncReadyWaitTime))
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
						 g_PhaseActionRad[i].phase = (CommandData[1]>>(2*i));
						 g_PhaseActionRad[i].actionRad =
								 pReciveFrame->pBuffer[2*i + 2] + ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
						 g_PhaseActionRad[i].enable = 0xFF;
						 g_PhaseActionRad[i].realRatio =   (float)g_PhaseActionRad[i].actionRad / 65536 ;
					 }
					 //��ֹ��բ�������
					 for (i = count; i < 3; i++)
					 {
						 g_PhaseActionRad[i].enable = 0;
					 }
					 g_PhaseActionRad[0].loopByte = CommandData[1];
					 memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
					 pSendFrame->pBuffer[0] = id| 0x80;
					 pSendFrame->len = pReciveFrame->len;
					 ZVDFlag = 0xFF; //����ͬ��Ԥ������
					 return 0;

				 }

			 }
			break;

		}
	}
	return 0xFF;



}
/**
 * ����֡����
 *
 * @param  pSendFrame ָ����֡��Ϣ��ָ��
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
