/***********************************************
*Copyright(c) 2017,FreeGo
*��������Ȩ��
*�ļ�����:CAN.c
*�ļ���ʶ:
*�������ڣ� 2017��3��6��
*ժҪ:
*2017/3/6 : ��ֲ����DeviceNet
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#include "DeviceNet.h"
#include "Timer.h"
#include "DSP28x_Project.h"
#include "CAN.h"
#include "Action.h"
#include "BasicModule.h"
#include "RefParameter.h"
#include "DeviceIO.h"

//DeviceNet ����ģʽ
#define  MODE_REPEAT_MAC  0xA1  //�ظ�MAC���
#define  MODE_NORMAL      0xA2  //��������ģʽ
#define  MODE_FAULT       0xA4  //����ģʽ
#define  MODE_STOP        0xA8  //ֹͣģʽ

////////////////////////////////////////////////////////////�ɿ��Ǵ���EEPROM
uint16_t  providerID = 0X1234;               // ��Ӧ��ID
uint16_t  device_type = 0;                   // ͨ���豸
uint16_t  product_code = 0X00d2;             // ��Ʒ����
uint8_t  major_ver = 0X01;
uint8_t  minor_ver = 0X01;                 // �汾
uint32_t  serialID = 0x001169BC;            // ���к�
SHORT_STRING  product_name = {8, (unsigned char *)"YongCi"};// ��Ʒ����
////////////////////////////////////////////////////////////////////////////////


//////////////////////��������/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config);

static void CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
static void AckCycleInquireMsgService(void);
void PacktIOMessageStatus( struct DefFrameData* pSendFrame);
BOOL IsTimeRemain();    //��Ҫ���ݾ���ƽ̨��д
void StartOverTimer();//��Ҫ���ݾ���ƽ̨��д
void SendData(struct DefFrameData* pFrame);//��Ҫ���ݾ���ƽ̨��д


////////////////////���Ӷ������////////////////////////////////
struct DefConnectionObj  CycleInquireConnedctionObj;//ѭ��IO��Ӧ
struct DefConnectionObj  StatusChangedConnedctionObj;//״̬�ı���Ӧ
struct DefConnectionObj  VisibleConnectionObj;   //��ʾ����
//////////////////DeviceNet�������////////////////////////////
struct DefDeviceNetClass  DeviceNetClass = {2}; //
struct DefDeviceNetObj  DeviceNetObj;
struct DefIdentifierObject  IdentifierObj; 

/**
 * DeviceNet�������ݱ�־������ѯ�����  bit1-0 = 11 ��ѯ����
 */
volatile uint16_t g_DeviceNetRequstData = 0;


//////////////////////�ļ�����///////////////////////////////////////////

uint8_t  SendBufferData[10];//���ջ�������
uint8_t  ReciveBufferData[10];//���ջ�������
struct DefFrameData  DeviceNetReciveFrame; //����֡����
struct DefFrameData  DeviceNetSendFrame; //����֡����

static volatile uint8_t WorkMode = 0; //

static volatile uint8_t StartTime = 0;


static RunTimeStamp LoopStatusSend;//ѭ��״̬����
static RunTimeStamp OffLine;// ��������״̬ʱ��ʱ��λ


/*******************************************************************************
* ������:	void InitDeviceNet()
* �β�  :	null
* ����ֵ:    null
* ��������:	��ʼ��DeviceNet���漰�Ļ�������
*******************************************************************************/
void InitDeviceNet()
{    
	ServiceDog();
    DeviceNetReciveFrame.complteFlag = 0xff;
    DeviceNetReciveFrame.pBuffer = ReciveBufferData;
    DeviceNetSendFrame.complteFlag = 0xff;
    DeviceNetSendFrame.pBuffer = SendBufferData;
  
    //////////��ʼ��DeviceNetObj����////////////////////////////////
	DeviceNetObj.MACID =0x0D ;                   //�������û�����ô�վ��ַ
	DeviceNetObj.baudrate = 2;                   //500Kbit/s
	DeviceNetObj.assign_info.select = 0;         //��ʼ������ѡ���ֽ�����
	DeviceNetObj.assign_info.master_MACID =0x02; //Ĭ����վ��ַ����Ԥ�����������ӽ��������У���վ������ߴ�վ����վ�ĵ�ַ
//////////////���Ӷ���Ϊ������״̬//////////////////////////
	VisibleConnectionObj.state =  STATE_NOT_EXIST ;
	CycleInquireConnedctionObj.state =  STATE_NOT_EXIST ;//״̬��û����վ���ӣ���վ��û�����ô�վ
    StatusChangedConnedctionObj.state = STATE_NOT_EXIST;
///////////////��ʼ����ʶ������///////////////
	IdentifierObj.providerID = providerID;        //providerID = 0X2620; ��Ӧ��ID
	IdentifierObj.device_type = device_type;      //device_type = 0;ͨ���豸
	IdentifierObj.product_code = product_code;    //product_code =0X00d2;��Ʒ����
	IdentifierObj.version.major_ver = major_ver;  //major_ver = 1;
	IdentifierObj.version.minor_ver = minor_ver;  //minor_ver = 1;�汾
	IdentifierObj.serialID = serialID;            //serialID = 0x001169BC;;���к�
	IdentifierObj.product_name = product_name;    //product_name = {8, "ADC4"};��Ʒ����
    WorkMode = MODE_REPEAT_MAC;
    g_CANErrorStatus = 0;
    BOOL result = CheckMACID( &DeviceNetReciveFrame, &DeviceNetSendFrame);
    ServiceDog();
    if (result)
    {
    	 WorkMode = MODE_FAULT;
    	 ON_LED3;
    	 OffLine.startTime =  CpuTimer0.InterruptCount;
    	 OffLine.delayTime = 10000;
    }
    else
    {
    	 OFF_LED3;
    	 WorkMode = MODE_NORMAL;
    	 g_DeviceNetRequstData = 0;//�����־��0
    }
    


}


/******************************************************************************* 
* ������:	void InitVisibleConnectionObj(void)
* ��  ��:	��
* ����ֵ:    	��
* ��������:	��ʽ��Ϣ�������ú���
********************************************************************************/   
void InitVisibleConnectionObj(void)
{
	VisibleConnectionObj.state = STATE_LINKED;	                //�ѽ���״̬
	VisibleConnectionObj.instance_type = 0x00;	        //��ʽ��Ϣ����
	VisibleConnectionObj.transportClass_trigger = 0x83;	//�������3
	VisibleConnectionObj.produced_connection_id = 0xFF;	
	VisibleConnectionObj.consumed_connection_id = 0xFF;	
	VisibleConnectionObj.initial_comm_characteristics = 0x22;	//ͨ����Ϣ��1����,ͨ����Ϣ��2����
	VisibleConnectionObj.produced_connection_size = 0xFF;	    
	VisibleConnectionObj.consumed_connection_size = 0xFF;		//������FF�ֽ�
	VisibleConnectionObj.expected_packet_rate = 0x09C4;		    //�����趨����������
	VisibleConnectionObj.watchdog_timeout_action = 1;		    //תΪ��ʱ״̬
	VisibleConnectionObj.produced_connection_path_length = 0;	
	VisibleConnectionObj.consumed_connection_path_length = 0xFF;	
	VisibleConnectionObj.produced_inhibit_time = 0;		        //��ʱ����
}
/******************************************************************************* 
* ������:	void InitCycleInquireConnectionObj(void)
* ����ֵ:    	��
* ��������:	I/O��ѯ�������ú���
* �β�:	��
********************************************************************************/    
void InitCycleInquireConnectionObj(void)
{
	CycleInquireConnedctionObj.state = STATE_LINKED;	                //����״̬
	CycleInquireConnedctionObj.instance_type = 0x01;	        //I/O����
	CycleInquireConnedctionObj.transportClass_trigger = 0x82;	//�������2
	CycleInquireConnedctionObj.produced_connection_id = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_id = 0xFF;	
	CycleInquireConnedctionObj.initial_comm_characteristics = 0x01;	//ͨ����Ϣ��1����,ͨ����Ϣ��2����
	CycleInquireConnedctionObj.produced_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.consumed_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.expected_packet_rate = 0;		        //�趨����������
	CycleInquireConnedctionObj.watchdog_timeout_action = 0;		    //תΪ��ʱ״̬
	CycleInquireConnedctionObj.produced_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.produced_inhibit_time = 0;		        //��ʱ����
}
/******************************************************************************* 
* ������:	void InitStatusChangedConnectionObj(void)
* ����ֵ:    	��
* ��������: ״̬�ı��������ú���
* �β�:	��
********************************************************************************/    
void InitStatusChangedConnectionObj(void)
{
	StatusChangedConnedctionObj.state = STATE_LINKED;	                //��������
	StatusChangedConnedctionObj.instance_type = STATUS_CHANGE;	       
	StatusChangedConnedctionObj.transportClass_trigger = 0x82;	//�������2
	StatusChangedConnedctionObj.produced_connection_id = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_id = 0xFF;	
	StatusChangedConnedctionObj.initial_comm_characteristics = 0x01;	//ͨ����Ϣ��1����,ͨ����Ϣ��2����
	StatusChangedConnedctionObj.produced_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.consumed_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.expected_packet_rate = 0;		        //�趨����������
	StatusChangedConnedctionObj.watchdog_timeout_action = 0;		    //תΪ��ʱ״̬
	StatusChangedConnedctionObj.produced_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.produced_inhibit_time = 0;		        //��ʱ����

    LoopStatusSend.startTime = CpuTimer0.InterruptCount;
    LoopStatusSend.delayTime = 3000; //3s �ϴ�һ��״̬��Ϣ
}
/*******************************************************************************  
** ������:	void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** �β�:	    struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:    	��
** ��������:	CAN��Ϣ��������������ȡ֡ID1��֡ID2�е���Ϣ��
                ������2�豸��������Ϣ���з��ദ��
*******************************************************************************/
void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    uint8_t mac = GET_GROUP2_MAC(pReciveFrame->ID);
    uint8_t function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
    ServiceDog();
	if(mac == DeviceNetObj.MACID)  //������2�豸
	{	        
        switch( function)
        {
            case GROUP2_REPEAT_MACID:  //�ظ�MAC ID�����Ϣ
            {          
                if (mac == DeviceNetObj.MACID)
                {
                    ResponseMACID(pReciveFrame, 0x80);       //�ظ�MACID�����Ӧ����,Ӧ������˿�Ϊ0
                }
                return; //MACID��ƥ��,����
            }
            case GROUP2_VSILBLE_ONLY2: //1100 0000����������ʾ������Ϣ��Ԥ������������
            {
            	ServiceDog();
				UnconVisibleMsgService(pReciveFrame, pSendFrame);    //��������ʽ��Ϣ����
                return;
            }
            case  GROUP2_POLL_STATUS_CYCLE: //��վI/O��ѯ����/״̬�仯/ѭ����Ϣ
            {     
            	ServiceDog();
                CycleInquireMsgService(pReciveFrame, pSendFrame);     // I/O��ѯ��Ϣ����
                return ;    
            }
            case GROUP2_VSILBLE:  //��վ��ʽ������Ϣ
            {               
				//VisibleMsgService(pReciveFrame, pSendFrame);        //��ʽ��Ϣ����
                break;
            }
            default:
            {
                
                break;
            }
        }
	}
}
/******************************************************************************
* ������:	void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config)
* �β�:	��
* ����ֵ:    	��
* ��������:	����ظ�MACID��Ӧ����
******************************************************************************/
void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config)
{                        //�ظ�MACID���
	 ServiceDog();
    pSendFrame->ID =  MAKE_GROUP2_ID( GROUP2_REPEAT_MACID, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = config;	                        //����/��Ӧ��־=1����ʾ��Ӧ���˿ں�0
	pSendFrame->pBuffer[1]= IdentifierObj.providerID;	//������ID���ֽ�
	pSendFrame->pBuffer[2] = IdentifierObj.providerID >> 8;	//������ID���ֽ�
	pSendFrame->pBuffer[3] = IdentifierObj.serialID;	                    //���кŵ��ֽ�
	pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 8;                //���к��м��ֽ�1
	pSendFrame->pBuffer[5] = IdentifierObj.serialID >>16;          //���к��м��ֽ�2
	pSendFrame->pBuffer[6] = IdentifierObj.serialID >>24;	//���кŸ��ֽ�
    pSendFrame->len = 7;
	SendData(pSendFrame);                      //���ͱ���
}
/*******************************************************************************
* ������:	BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* ��������:	��������ظ�MACID������
* �β�:	��
* ����ֵ:       TRUE    �������к��Լ��ظ��ĵ�ַ
                FALSE   ������û�к��Լ��ظ��ĵ�ַ
*******************************************************************************/
BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{	
    int sendCount = 0; 
    do
    {
    	ServiceDog();
        pReciveFrame->complteFlag = 0;
           //��������
        ResponseMACID( pSendFrame, 0);
    	if (g_CANErrorStatus != 0) //ͨѶ����
    	{
    		return TRUE;
    	}
        StartOverTimer();//������ʱ��ʱ��
        while(IsTimeRemain())
        {
        	ServiceDog();
            if ( pReciveFrame->complteFlag)//�ж��Ƿ���δ���͵�����
            {
                uint8_t mac = GET_GROUP2_MAC(pReciveFrame->ID);
                uint8_t function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
                if (function == GROUP2_REPEAT_MACID)
                {                  
                    if (mac == DeviceNetObj.MACID)
                    {
                         pReciveFrame->complteFlag = 0;
                         return TRUE; //ֻҪ��MACIDһ�£�����Ӧ���Ƿ���������Ϊ�ظ�
                    }
                }                
                else
                {
                    continue;
                }
            }
        }
      
    }
    while(++sendCount < 2);
    pReciveFrame->complteFlag = 0;
	return FALSE;	//û���ظ���ַ
}
/********************************************************************************
** ������:	void CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** �β�:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:      uint8_t 0-��� ����Ϊ���ͨ��
** ��������:j����������ʽ��Ϣ�����������Ӵ���
********************************************************************************/
BOOL CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    uint8_t error = 0; //����
    uint8_t errorAdd = 0; //���Ӵ�������
    ServiceDog();
    //����ѷ�����վ,�����Ƿ�����ͬһ��վ
    if((IdentifierObj.device_state & 0x01) && (pReciveFrame->pBuffer[5] != DeviceNetObj.assign_info.master_MACID))	//��֤��վ
    {	//�������Ե�ǰ��վ��������Ӧ
        error =  ERR_OBJECT_STATE_INFLICT;
        errorAdd = 0x01;     
    }
    //������2��������Ϣ���������ָ��DeviceNet��������IDΪ3����ÿ��DeviceNet������������ֻ��һ��DeviceNet���ʵ�������ʵ��IDΪ1
    else if(pReciveFrame->pBuffer[2] != 3 || pReciveFrame->pBuffer[3] != 1)	//��֤��ID��ʵ��ID
    {   //��֤��ID��ʵ��ID���󣬴�����Ӧ
        error =   ERR_PROPERTY_VALUE_INAVAIL;
        errorAdd = ERR_NO_ADDITIONAL_DESC;         
    }
    else if(pReciveFrame->pBuffer[4] == 0)	//��֤����ѡ���ֽ�
    {//�����ֽ�Ϊ�㣬��վû�����ô�վ��������Ӧ
        error =  ERR_PROPERTY_VALUE_INAVAIL; //(0x80 | ERR_PROPERTY_VALUE_INAVAIL)?
        errorAdd = 0x02;    
    }
    else if(pReciveFrame->pBuffer[4] & ~(CYC_INQUIRE | VISIBLE_MSG | BIT_STROKE|STATUS_CHANGE ))  
    {//���������ѯ���á���ʾ���ӡ�λѡͨ��������Ӧ
        error =  ERR_RES_INAVAIL;
        errorAdd = 0x02;        
    }
    if (error != 0)
    {
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pBuffer[2] = error;
        pSendFrame->pBuffer[3] = errorAdd;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);  //���ͱ���
        return FALSE;
    }
    return TRUE;
}
    
/********************************************************************************
** ������:	void CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** �β�:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:      uint8_t 0-��� ����Ϊ���ͨ��
** ��������:����������ʽ��Ϣ�����ͷ����Ӵ���
********************************************************************************/
BOOL CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    uint8_t error = 0; //����
    uint8_t errorAdd = 0; //���Ӵ�������
    uint8_t config = pReciveFrame->pBuffer[4];
    if(config == 0)   //��������ֽ�Ϊ0
    {	
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);      
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);
        return FALSE;
    }
    if(config & ~(CYC_INQUIRE | VISIBLE_MSG |BIT_STROKE|STATUS_CHANGE))//��֧�ֵ����ӣ�������Ӧ
    {
        error = ERR_RES_INAVAIL;
        errorAdd = 0x02;
    }
    else if((config & DeviceNetObj.assign_info.select) == 0)//���Ӳ����ڣ�������Ӧ
    {       
        error = ERR_EXISTED_MODE;
        errorAdd = 0x02;       
    }
     if (error != 0)
    {
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pBuffer[2] = error;
        pSendFrame->pBuffer[3] = errorAdd;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);  //���ͱ���
        return FALSE;
    }
    return TRUE;
}


/********************************************************************************
** ������:	void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** �β�:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:      ��
** ��������:	��������ʽ��Ϣ����������վ�øñ��������վ��������
********************************************************************************/
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{ 
	ServiceDog();
    if(pReciveFrame->pBuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pBuffer[1]���յ��ķ������
	{
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //���δͨ������
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pBuffer[5];  //��վ���ߴ�վ����վ�ĵ�ַ
        uint8_t config = pReciveFrame->pBuffer[4];
		DeviceNetObj.assign_info.select |= config;       //�����ֽ�
        
		ServiceDog();
		if(config & CYC_INQUIRE)                          //����I/O��ѯ����
		{	
			InitCycleInquireConnectionObj();                       //I/O��ѯ�������ú���
			CycleInquireConnedctionObj.produced_connection_id = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			CycleInquireConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
 	        //�ɹ�ִ����Ӧ
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // Ŀ��MAC ID(��վID)
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //��Ϣ���ʽ0,8/8��Class ID = 8 λ������Instance ID = 8 λ����
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
			SendData(pSendFrame);             //���ͱ���
			return ;
		}
		ServiceDog();
		if(config & VISIBLE_MSG)
		{	
			InitVisibleConnectionObj();//������ʽ��Ϣ����
			VisibleConnectionObj.produced_connection_id =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
			VisibleConnectionObj.consumed_connection_id =  MAKE_GROUP2_ID(GROUP2_VSILBLE, DeviceNetObj.MACID);
			//�ɹ�ִ����Ӧ
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	//��Ϣ���ʽ0,8/8����Class ID = 8 λ������ʵ��Instance ID = 8 λ����
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
			//����
			SendData(pSendFrame);			
		}
		ServiceDog();
		if(config & STATUS_CHANGE)                          //��������
		{
			InitStatusChangedConnectionObj();                       //״̬�ı��������ú���
			StatusChangedConnedctionObj.produced_connection_id = MAKE_GROUP1_ID( GROUP1_STATUS_CYCLE_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			StatusChangedConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
			//�ɹ�ִ����Ӧ
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // Ŀ��MAC ID(��վID)
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //��Ϣ���ʽ0,8/8��Class ID = 8 λ������Instance ID = 8 λ����
			pSendFrame->len = 3;
			pReciveFrame->complteFlag = 0;
			SendData(pSendFrame);             //���ͱ���

			return ;
		}

		IdentifierObj.device_state |= 0x01;	//�豸�Ѻ���վ����
		return ;
	}
	else if(pReciveFrame->pBuffer[1] == SVC_RELEASE_GROUP2_IDENTIFIER_SET)   //�ͷ����ӷ���
	{
		if(!CheckReleaseCode(pReciveFrame, pSendFrame))
        {
            return;
        }
		//�ͷ�����
        uint8_t config = pReciveFrame->pBuffer[4];
        
		DeviceNetObj.assign_info.select |= (config^0xff); //ȡ���ͷ���Ӧ������
        if(config & CYC_INQUIRE)    
        {
            CycleInquireConnedctionObj.produced_connection_id = 0;
			CycleInquireConnedctionObj.consumed_connection_id = 0;
            CycleInquireConnedctionObj.state = STATE_NOT_EXIST ;	
        }
        if(config & STATUS_CHANGE)    
        {
            StatusChangedConnedctionObj.produced_connection_id = 0;
			StatusChangedConnedctionObj.consumed_connection_id = 0;
            StatusChangedConnedctionObj.state = STATE_NOT_EXIST ;	
        }
        if (config & VISIBLE_MSG)
        {
            VisibleConnectionObj.produced_connection_id =  0;
			VisibleConnectionObj.consumed_connection_id =  0;
            VisibleConnectionObj.state = STATE_NOT_EXIST ;	
        }
		//ִ�гɹ���Ӧ
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
		pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
		pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);		
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
		SendData(pSendFrame);
	}
	else
	{	//��2��������ʽ��Ϣ����֧�֣�������Ӧ
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
		pSendFrame->pBuffer[3] = 0x02;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
		SendData(pSendFrame);
        
		return ;
	}
}

/********************************************************************************
** ������:	void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** ��������:	I/O��ѯ��Ϣ������������վ�ʹ�վ֮�䴫������
** �β�:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:      ��
*********************************************************************************/
static void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	ServiceDog();
    if(CycleInquireConnedctionObj.state != STATE_LINKED )	//��ѯI/O����û����
		return ;
    g_DeviceNetRequstData |= 0x0003; //��λ�����־
	return ;

}

/**
 * Ӧ��ѭ���������
 * @brief   Ӧ��ѭ��֡����
 */
static void AckCycleInquireMsgService(void)
{

	ServiceDog();
	uint8_t result = 0;
	//�������������������µ�֡
	result = FrameServer(&DeviceNetReciveFrame, &DeviceNetSendFrame);
	if (result !=0)
	{
		DeviceNetSendFrame.pBuffer[0] = 0x14;
		DeviceNetSendFrame.pBuffer[1] = DeviceNetReciveFrame.pBuffer[0];
		DeviceNetSendFrame.pBuffer[2] = result;
		DeviceNetSendFrame.pBuffer[3] = 0xFF;
		DeviceNetSendFrame.len = 4;
	}
	DeviceNetReciveFrame.complteFlag = 0;
	ServiceDog();
	PacktIOMessage(&DeviceNetSendFrame);
}
/**
 * �Է���IO���ݽ��д����ʹ����ѯ
 * @brief   Ӧ��ѭ��֡����
 */
void PacktIOMessage( struct DefFrameData* pSendFrame)
{
	ServiceDog();
	if (pSendFrame->len == 0)
	{
		return;
	}
	pSendFrame->ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
	SendData(pSendFrame);
}
/**
 * �Է���IO���ݽ��д��,ʹ��״̬�仯��Ϣ
 * @brief   Ӧ��ѭ��֡����
 */
void PacktIOMessageStatus( struct DefFrameData* pSendFrame)
{
	ServiceDog();
	if (pSendFrame->len == 0)
	{
		return;
	}
	pSendFrame->ID =  MAKE_GROUP1_ID(GROUP1_STATUS_CYCLE_ACK, DeviceNetObj.MACID);
	SendData(pSendFrame);
}
/*******************************************************************************
** ������:	void DeviceMonitorPluse(void)
** ��������:	�豸������庯��
** �β�:	��
** ����ֵ:      ��
********************************************************************************/
void DeviceMonitorPluse(void)
{
//	if(query_time_event(2))
//	{
//		start_time(160);
//		//���ͼ������
//		pSendFrame->pBuffer[0] = 0x80 | DeviceNetObj.MACID;
//		pSendFrame->pBuffer[1] = 0x60;
//		pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
//		pSendFrame->pBuffer[3]= 0x80 | SVC_MONITOR_PLUSE;
//		pSendFrame->pBuffer[4] = 0x01;		//��ʶ������ID=1
//		pSendFrame->pBuffer[5] = 0x00;
//		*(send_buf + 6) = IdentifierObj.device_state;
//		*(send_buf + 7) = 0;
//		*(send_buf + 8) = 0;
//		*(send_buf + 9) = 0;
//		SendData(10, send_buf);
//	}
}


/*******************************************************************************
* ������:	void DeviceNetReciveCenter(uint16* id, uint8 * pdata)
* ��������: �ӻ����л�ȡ���ݲ�����
* ��  ��: uint16* pID  11bitID��ʶ, uint8 * pbuff ��������, uint8 len ���ݳ���
* ����ֵ:      uint8_t  0-��Ϣδ���д���  1-��Ϣ���д���
********************************************************************************/
BOOL DeviceNetReciveCenter(uint16_t* pID, uint8_t * pbuff,uint8_t len)
{   
    uint8_t i= 0;
    //�ж��Ƿ�Ϊ������2---�������˲�����������
    ServiceDog();
    if( ((*pID) & 0x0600) != 0x0400)  //���ǽ�����2���Ĵ���
	{

    	SynCloseWaitAck(pID, pbuff, len);
        return FALSE;    
    }        
    
    if( DeviceNetReciveFrame.complteFlag) //
    {
        return FALSE;
    }
   
    if (len <= 8) //��󳤶�����
    {
         DeviceNetReciveFrame.ID = *pID;   
         DeviceNetReciveFrame.len = len;
        for(i = 0; i< len; i++) //��������
        {
            DeviceNetReciveFrame.pBuffer[i] = pbuff[i];
        }
        DeviceNetReciveFrame.complteFlag = 0xff;
         
        ServiceDog();
        switch(WorkMode)
        {
            case MODE_NORMAL: //��������ģʽ
            {
                CANFrameFilter(&DeviceNetReciveFrame, &DeviceNetSendFrame);
                break;
            }        
        }
         DeviceNetReciveFrame.complteFlag = 0;// Ĭ�ϴ˴��������
         return TRUE;
    }
    
    
    
    
    return 0;
}
/*******************************************************************************
* ������:	void  SendData(uint16* id, uint8 * pdata)----���ݾ���ƽ̨��Ҫ����
* ��������: �ӻ����л�ȡ���ݲ�����
* ��  ��: struct DefFrameData* pFrame
* ����ֵ:      null
********************************************************************************/
void SendData(struct DefFrameData* pFrame)
{

	  CANSendData(pFrame->ID, pFrame->pBuffer, pFrame->len);
      pFrame->complteFlag = 0;
}
/*******************************************************************************
* ������:	void  StartOverTimer()----���ݾ���ƽ̨��Ҫ����
* ��������: ������ʱ��ʱ��
* ��  ��:   null
* ����ֵ:   null
********************************************************************************/

void StartOverTimer()
{
	StartTime = CpuTimer0.InterruptCount;
}
/*******************************************************************************
* ������:	BOOL IsTimeRemain()----���ݾ���ƽ̨��Ҫ����
* ��������: ������ʱ��ʱ��
* ��  ��:   null
* ����ֵ:   TRUE-û�г�ʱ FALSE-��ʱ
********************************************************************************/
BOOL IsTimeRemain()
{
	 if (IsOverTime(StartTime, 1000))
	    {
	        return FALSE;
	    }
	    return TRUE;


}
/**
 * ͨѶ��˸����
 */
uint32_t flashComCn = 0;

/**
 * Ӧ���������--�ǽ��������
 * @brief   Ӧ��ѭ��֡����
 */
void AckMsgService(void)
{
	ServiceDog();
	if (WorkMode == MODE_FAULT) //
	{
		ON_LED3;
		//TODO: ���ͨѶ��������򣬳�ʱ��λ��
		if (IsOverTime(OffLine.startTime, OffLine.delayTime))
		{
			//�������½��г�ʼ��
			InitStandardCAN(0, 0);
			InitDeviceNet();
		}
		return;
	}
	//�д���
	if (  g_CANErrorStatus != 0)
	{
		ON_LED3;
		WorkMode =  MODE_FAULT; //��Ϊ����״̬
		OffLine.startTime =  CpuTimer0.InterruptCount; //���������µ���ʱ
		OffLine.delayTime = 5000;

	}
	ServiceDog();
	//����ͨѶָʾ��
	if (flashComCn++ >200000)
	{
		TOGGLE_LED3;
		flashComCn = 0;
	}
	//�Ѿ�������״̬�ı�����---�����Ա���״̬/����ͻ������--��Ԥ��״̬
	if ((StatusChangedConnedctionObj.state == STATE_LINKED) && ( g_SynCommandMessage.synActionFlag == 0))
	{
		if(IsOverTime(LoopStatusSend.startTime, LoopStatusSend.delayTime) )
		{

			DeviceNetSendFrame.pBuffer[0] = 0x1A | 0x80;
			DeviceNetSendFrame.pBuffer[1] = g_WorkMode;  //����ģʽ
			DeviceNetSendFrame.pBuffer[2] = CheckVoltageStatus();//��ѹԽ���Ƽ��
			DeviceNetSendFrame.pBuffer[3] = CheckFrequencyStatus();
			DeviceNetSendFrame.len = 4;
			PacktIOMessageStatus(&DeviceNetSendFrame);
			LoopStatusSend.startTime =  CpuTimer0.InterruptCount; //���������µ���ʱ
		}


	}
	//ͬ����բԤ��--�ȴ�Ӧ��/�ȴ�ִ��
	if( (g_SynCommandMessage.synActionFlag == SYN_HE_READY) ||
			(g_SynCommandMessage.synActionFlag == SYN_HE_WAIT_ACTION))
	{
		//����Ƿ�ʱ
		if (IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime, g_SynCommandMessage.closeWaitAckTime.delayTime))
		{
			g_SynCommandMessage.synActionFlag = 0;
			g_PhaseActionRad[0].readyFlag = 0;
			g_PhaseActionRad[1].readyFlag = 0;
			g_PhaseActionRad[2].readyFlag = 0;

		}
	 }

	//�Ƿ�����������
	if(g_DeviceNetRequstData == 0)
	{
		return;
	}
	if ((g_DeviceNetRequstData & 0x0003)==0x0003)//��ѯ��Ϣ
	{
		ServiceDog();
		AckCycleInquireMsgService();
		g_DeviceNetRequstData &= 0xFFFC; //�����־λ
	}





}
