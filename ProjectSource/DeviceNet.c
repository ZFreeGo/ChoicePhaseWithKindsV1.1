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
UINT  providerID = 0X1234;               // ��Ӧ��ID
UINT  device_type = 0;                   // ͨ���豸
UINT  product_code = 0X00d2;             // ��Ʒ����
USINT  major_ver = 0X01;
USINT  minor_ver = 0X01;                 // �汾
UDINT  serialID = 0x001169BC;            // ���к�
SHORT_STRING  product_name = {8, (unsigned char *)"YongCi"};// ��Ʒ����
////////////////////////////////////////////////////////////////////////////////


//////////////////////��������/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config);
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
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

BYTE  SendBufferData[10];//���ջ�������
BYTE  ReciveBufferData[10];//���ջ�������
struct DefFrameData  DeviceNetReciveFrame; //����֡����
struct DefFrameData  DeviceNetSendFrame; //����֡����

static volatile USINT WorkMode = 0; //

static volatile uint8_t StartTime = 0;


static RunTimeStamp LoopStatusSend;//ѭ��״̬����



/*******************************************************************************
* ������:	void InitDeviceNet()
* �β�  :	null
* ����ֵ:    null
* ��������:	��ʼ��DeviceNet���漰�Ļ�������
*******************************************************************************/
void InitDeviceNet()
{    
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
    BOOL result = CheckMACID( &DeviceNetReciveFrame, &DeviceNetSendFrame);
    
    if (result)
    {
    	 WorkMode = MODE_FAULT;
    	 ON_LED3;
    }
    else
    {
    	 WorkMode = MODE_NORMAL;
    	 g_DeviceNetRequstData = 0;//�����־��0
    }
    


}

/*******************************************************************************
* ������:	void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* �β�  :	struct DefFrameData * pFrame�����ձ�������
* ����ֵ:    	��
* ��������:	DeviceNet�������
            DeviceNet��ֻ��1�����ԣ���ѡִ��Get_Attribute_Single������Ӧ��汾��Ϣ
*******************************************************************************/
void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    
	if(pReciveFrame->pBuffer[1] != SVC_GET_ATTRIBUTE_SINGLE)        //��֧�ֵķ���
	{	
        //��2��Ϣ+ԴMAC ID(��վID)
        //��Ϣid=3����ʾ��վ��Ӧ��վ����ʾ����
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);        //
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);        //Ŀ��MAC ID(��վID)
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;  //R/R=1��ʾ��Ӧ��SVC_ERROR_RESPONSE��������Ӧ�������
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;    //ERR_SERVICE_NOT_SUPPORT ,��֧�ֵķ���
		pSendFrame->pBuffer[3] = ERR_RES_INAVAIL;            //ERR_RES_INAVAIL ,����ִ�з������Դ������(���Ӵ������)
		pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        //����
		SendData(pSendFrame);                    //���ʹ�����Ӧ����
		return ;
	}
	if(pReciveFrame->pBuffer[2] != 1)                               //�����ڵ�����
	{	
	    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
		pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;             //ERR_ID_INAVAIL ,������ָ������/ʵ��/����ID(���Ӵ������)
		pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;     //ERR_NO_ADDITIONAL_DESC ,�޸�����������
        pSendFrame->len = 4;
         pReciveFrame->complteFlag = 0;
		//����
		SendData(pSendFrame);                    //���ʹ�����Ӧ����
		return ;
	}
	//ִ����ʾ����
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);            //Ŀ��MAC ID(��վID)
	pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1��ʾ��Ӧ��VC_Get_Attribute_Single�������
	pSendFrame->pBuffer[2] = DeviceNetClass.version;
	pSendFrame->pBuffer[3] = DeviceNetClass.version >> 8;   //��İ汾��Ϣ
    pSendFrame->len = 4;
     pReciveFrame->complteFlag = 0;
	//����
	SendData(pSendFrame);                        //������ʾ��Ϣ����Ӧ����
}
/******************************************************************************** 
*������ : void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
*��  �� : struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����ձ�������
*����ֵ :  	��
*��������: DeviceNet���������
********************************************************************************/
void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
   
    switch( pReciveFrame->pBuffer[1])
    {
    case (SVC_GET_ATTRIBUTE_SINGLE):         //��ȡ�������Է���
	{        
        switch(pReciveFrame->pBuffer[4]) //����ID
        {
            case DEVICENET_OBJ_MACID:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);              
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);    //Ŀ��MAC ID(��վID)
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1��ʾ��Ӧ��SVC_GET_ATTRIBUTE_SINGLE�������
                pSendFrame->pBuffer[2] = DeviceNetObj.MACID;    //ԴMAC ID(��վID)
                pSendFrame->len = 3;
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);                //������ʾ��Ϣ����Ӧ����
                return ;             
            }
            case  DEVICENET_OBJ_BAUD:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                pSendFrame->pBuffer[2] = DeviceNetObj.baudrate;//��վ������
                pSendFrame->len = 3;
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);
                return;
            }
            case DEVICENET_OBJ_MASTERID:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.select;       //����ѡ��
                pSendFrame->pBuffer[3] = DeviceNetObj.assign_info.master_MACID; //��վMAC ID
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);
                return ;
            }
            default:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);       //Ŀ��MAC ID(��վID)
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE; //R/R=1��ʾ��Ӧ��SVC_ERROR_RESPONSE��������Ӧ�������
                pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;            //ERR_ID_INAVAIL ,������ָ������/ʵ��/����ID(���Ӵ������)
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;    //ERR_NO_ADDITIONAL_DESC ,�޸�����������
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);                   //���ʹ�����Ӧ����
                return ;
            }        
        }  
        break;
	}
	case ( SVC_SET_ATTRIBUTE_SINGLE):      //���õ������Է���
	{
        switch (pReciveFrame->pBuffer[3])
        {
            case DEVICENET_OBJ_MACID:
            case DEVICENET_OBJ_BAUD:
            case DEVICENET_OBJ_MASTERID://��֧�����ýڵ��ַ�������ʡ�����ѡ�����վMAC ID
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                pSendFrame->pBuffer[2] = ERR_PROPERTY_NOT_SET;
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);
                return ;              
            }
            default:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                pSendFrame->len = 4;
                
                pReciveFrame->complteFlag = 0;
                //����
                SendData(pSendFrame);
                return ;			           
            }
        }		
	}
	case (SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)://�������ӷ������
    case (SVC_RELEASE_GROUP2_IDENTIFIER_SET):  //�ͷ���2���ӷ���
	{	
		UnconVisibleMsgService(pReciveFrame, pSendFrame);                            //��������ʽ��Ϣ����
        break;
	}	
	default:
	{
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
		pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
		//����
		SendData(pSendFrame);
		return ;
	}
    }
}
/*******************************************************************************
* ������:	void ConnectionClassService(BYTE  *buf)
* ��  �� :	BYTE  *buf�����ձ�������
* ����ֵ:    	��
* ��������:	�����������
********************************************************************************/
void ConnectionClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);               //Ŀ��MAC ID(��վID)
	pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;         //R/R=1��ʾ��Ӧ��SVC_ERROR_RESPONSE��������Ӧ�������
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;           //ERR_SERVICE_NOT_SUPPORT ,��֧�ֵķ���
	pSendFrame->pBuffer[3] = 0x01;                              //���Ӵ������
    pSendFrame->len = 4;
    
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);//����
	return ;
}
/*******************************************************************************
* ������:	void VisibleConnectObjService(BYTE  *buf)
* �β�:	BYTE  *buf�����ձ�������
* ����ֵ:    	��
* ��������:	��ʽ��Ϣ���ӷ�����
*******************************************************************************/
void VisibleConnectObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);     // Ŀ��MAC ID(��վID)
    
    switch( pReciveFrame->pBuffer[1])
    {
        case SVC_SET_ATTRIBUTE_SINGLE://���÷���#define SVC_SET_ATTRIBUTE_SINGLE	0x10
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_SET_ATTRIBUTE_SINGLE;// R/R=1��ʾ��Ӧ��pReciveFrame->pBuffer[1]�ǽ��ձ����еķ������
            pSendFrame->pBuffer[2] =  pReciveFrame->pBuffer[5]; //������Ҫ��ӳ�������
            pSendFrame->pBuffer[3] =  pReciveFrame->pBuffer[6];;
            pSendFrame->len = 4;
            break;       
        }
        case SVC_GET_ATTRIBUTE_SINGLE://��ȡ����
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
            switch(pReciveFrame->pBuffer[4])
            {
                case 7:                   //����IDΪ7����ʾͨ�������ӷ��͵�����ֽ���Ϊ8��
                {	 			
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //����IDΪ8����ʾͨ�������ӽ��յ�����ֽ���Ϊ8��
                { 
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                default:
                {
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
                    pSendFrame->pBuffer[3] = 0x01;
                    pSendFrame->len = 4;
                    break;
                }
            }
            break;
        }
        default:
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
            pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
            pSendFrame->pBuffer[3] = 0x01;
            pSendFrame->len = 4;
            break;
        }            
    }  	
	//����
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
}
/********************************************************************************
* ������:	void CycInquireConnectObjService(BYTE  *buf)
* ��  ��:	BYTE  *buf�����ձ�������
* ����ֵ:    	��
* ��������:	��ѯ��Ϣ����ʵ��������
********************************************************************************/
void CycInquireConnectObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);	
    switch(pReciveFrame->pBuffer[1])
    {
        case  SVC_SET_ATTRIBUTE_SINGLE:
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_SET_ATTRIBUTE_SINGLE;     
            switch (pReciveFrame->pBuffer[4]) //�������
            {
                case 1: //state ����  4-��ʱ 3-�ѽ��������� 1-����״̬
                {                  
                    if (pReciveFrame->pBuffer[5] <= 4) //����Ƿ�Ϊ��Ч״̬
                    {
                        //TODO:��Ҫ��һ�����״̬--״̬���ת��
                        CycleInquireConnedctionObj.state = pReciveFrame->pBuffer[5]; 
                        pSendFrame->pBuffer[2] = CycleInquireConnedctionObj.state;
                        pSendFrame->len = 3;
                    }
                    else
                    {
                        pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                        pSendFrame->pBuffer[2] =  ERR_PROPERTY_VALUE_INAVAIL;
                        pSendFrame->pBuffer[3] = 0xFF;
                        pSendFrame->len = 4;
                    }
                    break;                                      
                }
            }
            break;
        }
        case SVC_GET_ATTRIBUTE_SINGLE:
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
            switch(pReciveFrame->pBuffer[4])
            {
                case 7:                   //����IDΪ7����ʾͨ�������ӷ��͵�����ֽ���Ϊ8��
                {	 			
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //����IDΪ8����ʾͨ�������ӽ��յ�����ֽ���Ϊ8��
                { 
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                default:
                {
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
                    pSendFrame->pBuffer[3] = 0x01;
                    pSendFrame->len = 4;
                    break;
                }
            }
            break;
        }
        default:
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
            pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
            pSendFrame->pBuffer[3] = 0x01;
            pSendFrame->len = 4;
            break;
        }
    }
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
}

/*******************************************************************************
* ������:	void IdentifierClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* �β�:	BYTE  *buf�����ձ�������
* ����ֵ:    	��
* ��������:	��ʶ�������, ��֧���κη��񣬴�����Ӧ
********************************************************************************/
void IdentifierClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
	pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
	pSendFrame->pBuffer[3] = 0x01;
    pSendFrame->len = 4;
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
	return ;
}
/*******************************************************************************
* ������:	void IdentifierObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* ��  ��:	BYTE  *buf�����ձ�������
* ����ֵ:    	��
* ��������:	��ʶ���������������Ӧ��վ�йر�ʾ��������
********************************************************************************/   
void IdentifierObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
    pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F; 
   
	if(pReciveFrame->pBuffer[1] == SVC_GET_ATTRIBUTE_SINGLE)
	{	 
		pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;        
        USINT attribute = pReciveFrame->pBuffer[4];
        pReciveFrame->complteFlag = 0;
        
        switch(attribute)
        {
            case IDENTIFIER_OBJ_SUPPLIERID:  //����ID1����ʾ�����ֱ�ʶ����Ӧ��
            {
                pSendFrame->pBuffer[2] = IdentifierObj.providerID;
			    pSendFrame->pBuffer[3] = IdentifierObj.providerID >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);                
                break;
            }
            case IDENTIFIER_OBJ_TYPE://����ID2����ʾ��Ʒͨ������˵��
            {
                pSendFrame->pBuffer[2] = IdentifierObj.device_type;
			    pSendFrame->pBuffer[3] = IdentifierObj.device_type >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_CODE://����ID3����ʾ��Ʒ����
            {
                pSendFrame->pBuffer[2] = IdentifierObj.product_code;
			    pSendFrame->pBuffer[3] = IdentifierObj.product_code >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_VERSION://����ID4����ʾ��Ʒ�汾
            {
                pSendFrame->pBuffer[2] = IdentifierObj.version.major_ver;
			    pSendFrame->pBuffer[3] = IdentifierObj.version.minor_ver;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
             case IDENTIFIER_OBJ_STATUES://����ID5����ʾ�豸״̬����
             {
                 pSendFrame->pBuffer[2] = IdentifierObj.device_state;
			     pSendFrame->pBuffer[3] = IdentifierObj.device_state >> 8;
                 pSendFrame->len = 4;
			     SendData(pSendFrame);
                 break;
             }
            case IDENTIFIER_OBJ_SERIALNUM://����ID6����ʾ�豸���к�
            {
             	pSendFrame->pBuffer[2] = IdentifierObj.serialID;
			    pSendFrame->pBuffer[3] = IdentifierObj.serialID >> 8;
                pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 16;
                pSendFrame->pBuffer[5] = IdentifierObj.serialID >> 24;
                pSendFrame->len = 6;
                SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_NAME://����ID7����ʾ��Ʒ����
            {
                pSendFrame->pBuffer[2] = IdentifierObj.product_name.length;
                pSendFrame->pBuffer[3] = IdentifierObj.product_name.ucdata[0];
                pSendFrame->pBuffer[4] = IdentifierObj.product_name.ucdata[1];
                pSendFrame->pBuffer[5]= IdentifierObj.product_name.ucdata[2];
                pSendFrame->pBuffer[6] = IdentifierObj.product_name.ucdata[3];
                pSendFrame->pBuffer[7] = IdentifierObj.product_name.ucdata[4];
                pSendFrame->len = 8;
                SendData(pSendFrame);
                break;
            }
            default://��֧�ֵķ���
            {            
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                pSendFrame->pBuffer[2] = ERR_PROPERTY_NOT_SET;
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                pSendFrame->len = 4;
                SendData(pSendFrame);
                break;
            }
        }
    }
    else
    {
        pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
        pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
        pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
        pSendFrame->len = 4;	
        SendData(pSendFrame);
    }   
}
/********************************************************************************
* ������:	void RountineClassObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* �β�:	��
* ����ֵ:    	��
* ��������:	��Ϣ·�������񣬲�֧���κη��񣬴�����Ӧ
********************************************************************************/  
void RountineClassObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
	pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
	pSendFrame->pBuffer[3] = 0x01;
    pSendFrame->len = 4;
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
	return ;	
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
    BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
    BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
	
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
				UnconVisibleMsgService(pReciveFrame, pSendFrame);    //��������ʽ��Ϣ����
                return;
            }
            case  GROUP2_POLL_STATUS_CYCLE: //��վI/O��ѯ����/״̬�仯/ѭ����Ϣ
            {     
                CycleInquireMsgService(pReciveFrame, pSendFrame);     // I/O��ѯ��Ϣ����
                return ;    
            }
            case GROUP2_VSILBLE:  //��վ��ʽ������Ϣ
            {               
				VisibleMsgService(pReciveFrame, pSendFrame);        //��ʽ��Ϣ����
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
* ������:	void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
* �β�:	��
* ����ֵ:    	��
* ��������:	����ظ�MACID��Ӧ����
******************************************************************************/
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
{                        //�ظ�MACID���
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
        pReciveFrame->complteFlag = 0;
           //��������
        ResponseMACID( pSendFrame, 0);
    	if (g_CANErrorStatus != 0) //ͨѶ����
    	{
    		return TRUE;
    	}
        StartOverTimer();//������ʱ��ʱ��
        while( IsTimeRemain())
        {
            if ( pReciveFrame->complteFlag)//�ж��Ƿ���δ���͵�����
            {
                BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
                BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
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
** ����ֵ:      BYTE 0-��� ����Ϊ���ͨ��
** ��������:j����������ʽ��Ϣ�����������Ӵ���
********************************************************************************/
BOOL CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //����
    BYTE errorAdd = 0; //���Ӵ�������
    
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
** ����ֵ:      BYTE 0-��� ����Ϊ���ͨ��
** ��������:����������ʽ��Ϣ�����ͷ����Ӵ���
********************************************************************************/
BOOL CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //����
    BYTE errorAdd = 0; //���Ӵ�������
    USINT config = pReciveFrame->pBuffer[4];
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
    if(pReciveFrame->pBuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pBuffer[1]���յ��ķ������
	{
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //���δͨ������
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pBuffer[5];  //��վ���ߴ�վ����վ�ĵ�ַ
        USINT config = pReciveFrame->pBuffer[4];
		DeviceNetObj.assign_info.select |= config;       //�����ֽ�
        

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
		if(config &  BIT_STROKE) //����λѡͨ����
		{	
		
//			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
//			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
//			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
//			pSendFrame->pBuffer[2] = 0;	//��Ϣ���ʽ0,8/8
//            pSendFrame->len = 3;
//			SendData(pSendFrame);
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
        USINT config = pReciveFrame->pBuffer[4];
        
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
/*********************************************************************************
** ������:	void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** �β�:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame�����յı�������
** ����ֵ:      ��
** ��������:	��ʽ��Ϣ��������ִ����վ����ʾ������Ӧ
*********************************************************************************/
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	BYTE class, obj;

	class = pReciveFrame->pBuffer[2]; //��ID
	obj = pReciveFrame->pBuffer[3];   //ʵ��ID

	//��Ϣ·��
	if(!(DeviceNetObj.assign_info.select & VISIBLE_MSG))	//û�н�����ʽ��Ϣ����
		return ;
    
    switch(class)
    {
        case 0x01: //��ʶ������
        {   
            if(obj == 0)	    //�����
            {
                IdentifierClassService( pReciveFrame, pSendFrame);
            }
            else if(obj == 1)	//ʵ��1����
            {
                IdentifierObjService(pReciveFrame, pSendFrame);
            }              
            break;
        }
        case 0x02: //��Ϣ·��������
        {
            RountineClassObjService(pReciveFrame, pSendFrame);
            break;
        }
        case 0x03://DeviceNet����
        {
            if(obj == 0)	    //�����
            {
                DeviceNetClassService(pReciveFrame, pSendFrame);
                return ;
            }
            else if(obj == 1)	//ʵ��1����
            {
                DeviceNetObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }
        case 0x05:	//���Ӷ���
        {
            if(obj == 0)	    //�����
            {
                ConnectionClassService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 1)	//��ʽ��Ϣ����
            {
                VisibleConnectObjService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 2)	//I/O��ѯ����
            {
                CycInquireConnectObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }   
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


    if(CycleInquireConnedctionObj.state != STATE_LINKED )	//��ѯI/O����û����
		return ;

    g_DeviceNetRequstData |= 0x0003; //��λ�����־
   // result = FrameServer(pReciveFrame,  pSendFrame);
    //if (result !=0)
   // {
    //	   pSendFrame->pBuffer[0] = 0x14;
    //	   pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[0];
    //	   pSendFrame->pBuffer[2] = result;
    //	   pSendFrame->pBuffer[3] = 0xFF;
    //	   pSendFrame->len = 4;
    //}
   // pReciveFrame->complteFlag = 0;
    
   // pSendFrame->ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);

  	//SendData(pSendFrame);
	return ;
}

/**
 * Ӧ��ѭ���������
 * @brief   Ӧ��ѭ��֡����
 */
static void AckCycleInquireMsgService(void)
{
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

	PacktIOMessage(&DeviceNetSendFrame);
	//DeviceNetSendFrame.ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);

	//SendData(&DeviceNetSendFrame);
}
/**
 * �Է���IO���ݽ��д����ʹ����ѯ
 * @brief   Ӧ��ѭ��֡����
 */
void PacktIOMessage( struct DefFrameData* pSendFrame)
{
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
* ����ֵ:      BYTE  0-��Ϣδ���д���  1-��Ϣ���д���
********************************************************************************/
BOOL DeviceNetReciveCenter(UINT* pID, USINT * pbuff,USINT len)
{   
    BYTE i= 0;
    //�ж��Ƿ�Ϊ������2---�������˲�����������
    if( ((*pID) & 0x0600) != 0x0400)  //���ǽ�����2���Ĵ���
	{       
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
 * Ӧ���������--�ǽ��������
 * @brief   Ӧ��ѭ��֡����
 */
void AckMsgService(void)
{
	if (WorkMode== MODE_FAULT) //
	{
		ON_LED3;
		//TODO: ���ͨѶ��������򣬳�ʱ��λ��
		return;
	}
	//�Ѿ�������״̬�ı�����---�����Ա���״̬/����ͻ������
	if (StatusChangedConnedctionObj.state == STATE_LINKED)
	{
		if(IsOverTime(LoopStatusSend.startTime, LoopStatusSend.delayTime) )
		{
			DeviceNetSendFrame.pBuffer[0] = 0x1A | 0x80;
			DeviceNetSendFrame.pBuffer[1] = 0xA1; //����
			DeviceNetSendFrame.pBuffer[2] = 0xA2; //����
			DeviceNetSendFrame.pBuffer[3] = 0xA3; //����
			DeviceNetSendFrame.pBuffer[4] = 0xA4; //����
			DeviceNetSendFrame.pBuffer[5] = 0xA5; //����
			DeviceNetSendFrame.len = 6;
			PacktIOMessageStatus(&DeviceNetSendFrame);
			LoopStatusSend.startTime =  CpuTimer0.InterruptCount; //���������µ���ʱ
		}


	}

	if(g_DeviceNetRequstData == 0)
	{
		return;
	}
	if ((g_DeviceNetRequstData & 0x0003)==0x0003)//��ѯ��Ϣ
	{
		AckCycleInquireMsgService();
		g_DeviceNetRequstData &= 0xFFFC; //�����־λ
	}




}
