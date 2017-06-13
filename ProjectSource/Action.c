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
#include "Action.h"

/**
 * ͬ��Ԥ��������Ϣ
 */
RefSynCommandMessage g_SynCommandMessage;



uint8_t  SendBufferDataAction[10];//���ջ�������
struct DefFrameData  ActionSendFrame; //����֡����


/**
 * ��ʼ��ʹ�õ�����
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

static uint8_t SynHezha(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);

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


	//uint8_t count = 0;
	uint8_t i = 0;

	//uint8_t ph[3] = {0};//����ѡ��
	//uint16_t rad[3] = {0};//���ȹ�һ��ֵ
	uint8_t tempData[8] = {0};
	PointUint8 point;
	uint8_t result = 0;
	uint8_t codeStart = 0;
	uint8_t codeEnd = 0;

	//uint8_t remain = 0;
	//uint16_t temp = 0;
	ServiceDog();
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
	ServiceDog();
	switch (id)
	{

		case 0x11: //��վ��������
		{
			ServiceDog();
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
			ServiceDog();
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
					ServiceDog();
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
					ServiceDog();
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
			ServiceDog();
			if (pReciveFrame->len == 4) //ID+���ú� ����2�ֽ�
			{
				if (pReciveFrame->pBuffer[1] != g_MacList[0])
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
					pSendFrame->pBuffer[1] = g_MacList[0];
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
					memcpy(pSendFrame->pBuffer + 1, pReciveFrame->pBuffer + 1,
	 					 pReciveFrame->len - 1);
					pSendFrame->len = pReciveFrame->len;
					return 0;
				}

			}
			break;
		}
		case 0x1B://����һ֡���ݶ�ȡ�ṹ
		{
			ServiceDog();
			if (pReciveFrame->len >= 2) //ID+���ú� ����2�ֽ�
			{
				ServiceDog();
				if (pReciveFrame->pBuffer[1] != 0xAA)
				{
					return 0xE5;
				}
				SendMultiFrame(pSendFrame);

				pSendFrame->len = 0; //�õײ��ֹ����
				return 0;


			}


		}
		case 0x30: //ͬ����բԤ��
		case 0x31: //ͬ����բִ��
		{
			return SynHezha(pReciveFrame, pSendFrame);
		}

	}
	return 0xFF;



}
/**
 * ͬ����բ����--Ԥ����ִ��
 *
 * @param  ָ����֡��Ϣ���ݵ�ָ��
 * @param  ָ����֡��Ϣ��ָ��
 *
 * @retrun �������
 * @bref   ������֡������ȡ�ж�
 */
static uint8_t SynHezha(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	uint8_t id = 0;
	uint8_t count = 0;
	uint8_t i = 0;
	uint8_t ph[3] = {0};//����ѡ��
	uint16_t rad[3] = {0};//���ȹ�һ��ֵ
	float lastRatio = 0; //��һ�α���
	uint8_t phase = 0;
	ServiceDog();
	id = pReciveFrame->pBuffer[0];
	switch (id)
	{
		case 0x30://ͬ����բԤ��
		{
			ServiceDog();
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
			g_SynCommandMessage.loopByte = pReciveFrame->pBuffer[1];
			for(i = 0; i < count; i++)
			{
				ph[i] = (uint8_t)((g_SynCommandMessage.loopByte>>(2*i))&(0x03));
				rad[i] = pReciveFrame->pBuffer[2*i + 2] | ((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8;
			}
			if (count == 3)
			{
				//��ѡ������ͬ
				if ((ph[0] == ph[1]) || (ph[2] == ph[1]) || (ph[2] == ph[0]))
				{
					return 0XF5;
				}

				//���ȱ����Դ�����
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

			//����ͬ����բ����
			 count = (pReciveFrame->len - 2)/2;
			 lastRatio = 0;
			 for(i = 0; i < count; i++)
			 {
				 phase= (pReciveFrame->pBuffer[1]>>(2*i))&(0x03);

				 //phase λ��1-3֮��
				 if ((phase <= 3) && (phase >= 1))
				 {
					 g_PhaseActionRad[i].phase = phase - 1; //��1��Ϊ����
				 }
				 else
				 {
					 return 0xF1;
				 }
				 g_PhaseActionRad[i].actionRad =  pReciveFrame->pBuffer[2*i + 2] + (((uint16_t)pReciveFrame->pBuffer[2*i + 3])<<8);


				 g_PhaseActionRad[i].realRatio =  (float)g_PhaseActionRad[i].actionRad / 65536   ;//�ۼӼ�����Ա���

				 //�жϺ�һ����ǲ�С��ǰһ��
				 if (g_PhaseActionRad[i].realRatio < lastRatio)
				 {
					return 0xFC;
				 }
				 g_PhaseActionRad[i].realDiffRatio =   g_PhaseActionRad[i].realRatio  - lastRatio;//�����һ������
				 lastRatio = g_PhaseActionRad[i].realRatio;

				 g_PhaseActionRad[i].count = count;//��·����
			 }
			 //���ǽ�ֹ����ͬ��ִ��״̬�¿���
			 for (i = count; i < 3; i++)
			 {
				 g_PhaseActionRad[i].enable = 0;
			 }

			memcpy(g_SynCommandMessage.commandData, pReciveFrame->pBuffer, pReciveFrame->len );//�ݴ�ָ��
			g_SynCommandMessage.lastLen = pReciveFrame->len;
			g_SynCommandMessage.synActionFlag = SYN_HE_READY;//�ݴ�ָ���־
			g_SynCommandMessage.closeWaitAckTime.startTime = CpuTimer0.InterruptCount;
			memcpy(pSendFrame->pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
			pSendFrame->pBuffer[0] = id| 0x80;
			pSendFrame->len = pReciveFrame->len;
			return 0;


		}
		case 0x31://ͬ����բִ��
		{
			ServiceDog();
			 //�ж��Ƿ�ʱ
			 if (!IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime, g_SynCommandMessage.closeWaitAckTime.delayTime))
			 {
				 if (g_SynCommandMessage.synActionFlag == SYN_HE_WAIT_ACTION)//�Ƿ��Ѿ�Ԥ��
				 {
					 g_SynCommandMessage.synActionFlag = 0; //���Ԥ��
					 if (pReciveFrame->len != g_SynCommandMessage.lastLen)
					 {
						 return 0XF9;
					 }
					 //��һ��ָ���Ƿ�Ϊ��բԤ��
					 if (g_SynCommandMessage.commandData[i] != 0x30)
					 {
						 return 0XFA;
					 }
					 //�Ƚ�ִ��ָ����Ԥ��ָ���Ƿ���ͬ
					 for(i = 1; i < pReciveFrame->len;i++)
					 {
						if (g_SynCommandMessage.commandData[i] != pReciveFrame->pBuffer[i])
						{
							return 0XFB;
						}
					 }
					 //����ͬ����բ����
					 count = (pReciveFrame->len - 2)/2;
					 for(i = 0; i < count; i++)
					 {
						 g_PhaseActionRad[i].enable = 0xFF;
					 }
					 //��ֹ��բ�������
					 for (i = count; i < 3; i++)
					 {
						 g_PhaseActionRad[i].enable = 0;
					 }

					 g_PhaseActionRad[0].loopByte = g_SynCommandMessage.commandData[1];
					 memcpy(ActionSendFrame.pBuffer, pReciveFrame->pBuffer, pReciveFrame->len );
					 ActionSendFrame.pBuffer[0] = id| 0x80;
					 ActionSendFrame.len = pReciveFrame->len;
					 g_SynCommandMessage.synActionFlag = SYN_HE_ACTION;//��ͬ����բ������־
					 g_SynCommandMessage.synActionFlag = SYN_HE_ACTION;//��ͬ����բ������־
					 pSendFrame->len = 0;//ȡ���ײ㷢��
					 return 0;

				 }

			 }
			 else
			 {
				 g_SynCommandMessage.synActionFlag = 0;
				 for (i = count; i < 3; i++)
				 {
					 g_PhaseActionRad[i].enable = 0;
				 }
			 }
		}
	}
	return 0xFF;

}
/**
 * ͬ��ִ��Ӧ��
 *
 * @param  Ӧ��״̬ 0-�������ظ�����Ӧ�� ��0--һ�δ���
 *
 * @retrun null
 */
void SynActionAck(uint8_t state)
{
	ServiceDog();
	if (state != 0)
	{
		ActionSendFrame.pBuffer[0] = 0x14;
		ActionSendFrame.pBuffer[1] = 0x31;//ͬ��ִ��
		ActionSendFrame.pBuffer[2] = state;
		ActionSendFrame.pBuffer[3] = 0xFF;
		ActionSendFrame.len = 4;
	}
	PacktIOMessage( &ActionSendFrame);



}

/**
 * ͬ����բ�ȴ�Ӧ��״̬
 * �ж��Ƿ�Ϊ��վ�ڵ�Ӧ���ж��Ƿ���������բԤ��Ӧ����������Ϊ��Ӧ�ĺ�բԤ��״̬�����ȫ��Ԥ�����Ƿ���ȫ,
 * ����ȫ����ʱ��ȴ�Ԥ��ʱ��T2��
 *
 * @param  pID  11bitID��ʶ
 * @param  pbuff ��������
 * @param  len ���ݳ���
 *
 * @retrun 0--δ���д���  ��0--����Ҫ����д���
 */
uint8_t SynCloseWaitAck(uint16_t* pID, uint8_t * pbuff,uint8_t len)
{
	uint8_t i = 0, mac = 0;
	//�Ƿ�ΪԤ��״̬
	if ( g_SynCommandMessage.synActionFlag != SYN_HE_READY )
	{
		return 0;
	}
	ServiceDog();
	//�Ƿ�ʱ
	if (IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime,
					g_SynCommandMessage.closeWaitAckTime.delayTime))
	{
		//��λ
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
		//��ַ�Ƿ�����A,B,C��
		if (g_MacList[g_PhaseActionRad[i].phase + 1] == mac)
		{
			if (len < 2)
			{
				return 0xff;
			}
			if (pbuff[0] == (0x05 | 0x80))//��վ ͬ����բԤ�Ʒ���ָ��
			{
				g_PhaseActionRad[i].readyFlag = 0xff;

				//����Ƿ�ȫ����ΪԤ��״̬
				for(i = 0; i < g_PhaseActionRad[0].count; i++)
				{
					//��δ׼���࣬����
					if (g_PhaseActionRad[i].readyFlag == 0)
					{
						return 0xff;
					}
				}
				//ȫ����Ѿ�����
				g_SynCommandMessage.synActionFlag = SYN_HE_WAIT_ACTION;
				g_SynCommandMessage.closeWaitActionTime.startTime = CpuTimer0.InterruptCount;
				return  0xff;
			}
		}
	}
	return 0xff;







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




