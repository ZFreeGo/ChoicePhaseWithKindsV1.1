/***********************************************
*Copyright(c) 2017,FreeGo
*保留所有权利
*文件名称:CAN.c
*文件标识:
*创建日期： 2017年3月6日
*摘要:
*2017/3/6 : 移植部分DeviceNet
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "DeviceNet.h"
#include "Timer.h"
#include "DSP28x_Project.h"
#include "CAN.h"
#include "Action.h"
#include "BasicModule.h"
#include "RefParameter.h"
#include "DeviceIO.h"

//DeviceNet 工作模式
#define  MODE_REPEAT_MAC  0xA1  //重复MAC检测
#define  MODE_NORMAL      0xA2  //正常工作模式
#define  MODE_FAULT       0xA4  //故障模式
#define  MODE_STOP        0xA8  //停止模式

////////////////////////////////////////////////////////////可考虑存入EEPROM
UINT  providerID = 0X1234;               // 供应商ID
UINT  device_type = 0;                   // 通用设备
UINT  product_code = 0X00d2;             // 产品代码
USINT  major_ver = 0X01;
USINT  minor_ver = 0X01;                 // 版本
UDINT  serialID = 0x001169BC;            // 序列号
SHORT_STRING  product_name = {8, (unsigned char *)"YongCi"};// 产品名称
////////////////////////////////////////////////////////////////////////////////


//////////////////////函数申明/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config);
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
static void CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
static void AckCycleInquireMsgService(void);
void PacktIOMessageStatus( struct DefFrameData* pSendFrame);
BOOL IsTimeRemain();    //需要根据具体平台改写
void StartOverTimer();//需要根据具体平台改写
void SendData(struct DefFrameData* pFrame);//需要根据具体平台改写


////////////////////连接对象变量////////////////////////////////
struct DefConnectionObj  CycleInquireConnedctionObj;//循环IO响应
struct DefConnectionObj  StatusChangedConnedctionObj;//状态改变响应
struct DefConnectionObj  VisibleConnectionObj;   //显示连接
//////////////////DeviceNet对象变量////////////////////////////
struct DefDeviceNetClass  DeviceNetClass = {2}; //
struct DefDeviceNetObj  DeviceNetObj;
struct DefIdentifierObject  IdentifierObj; 

/**
 * DeviceNet请求数据标志，如轮询请求等  bit1-0 = 11 轮询请求
 */
volatile uint16_t g_DeviceNetRequstData = 0;


//////////////////////文件变量///////////////////////////////////////////

BYTE  SendBufferData[10];//接收缓冲数据
BYTE  ReciveBufferData[10];//接收缓冲数据
struct DefFrameData  DeviceNetReciveFrame; //接收帧处理
struct DefFrameData  DeviceNetSendFrame; //接收帧处理

static volatile USINT WorkMode = 0; //

static volatile uint8_t StartTime = 0;


static RunTimeStamp LoopStatusSend;//循环状态发送



/*******************************************************************************
* 函数名:	void InitDeviceNet()
* 形参  :	null
* 返回值:    null
* 功能描述:	初始化DeviceNet所涉及的基本数据
*******************************************************************************/
void InitDeviceNet()
{    
    DeviceNetReciveFrame.complteFlag = 0xff;
    DeviceNetReciveFrame.pBuffer = ReciveBufferData;
    DeviceNetSendFrame.complteFlag = 0xff;
    DeviceNetSendFrame.pBuffer = SendBufferData;
  
    //////////初始化DeviceNetObj对象////////////////////////////////
	DeviceNetObj.MACID =0x0D ;                   //如果跳键没有设置从站地址
	DeviceNetObj.baudrate = 2;                   //500Kbit/s
	DeviceNetObj.assign_info.select = 0;         //初始的配置选择字节清零
	DeviceNetObj.assign_info.master_MACID =0x02; //默认主站地址，在预定义主从连接建立过程中，主站还会告诉从站：主站的地址
//////////////连接对象为不存在状态//////////////////////////
	VisibleConnectionObj.state =  STATE_NOT_EXIST ;
	CycleInquireConnedctionObj.state =  STATE_NOT_EXIST ;//状态：没和主站连接，主站还没有配置从站
    StatusChangedConnedctionObj.state = STATE_NOT_EXIST;
///////////////初始化标识符对象///////////////
	IdentifierObj.providerID = providerID;        //providerID = 0X2620; 供应商ID
	IdentifierObj.device_type = device_type;      //device_type = 0;通用设备
	IdentifierObj.product_code = product_code;    //product_code =0X00d2;产品代码
	IdentifierObj.version.major_ver = major_ver;  //major_ver = 1;
	IdentifierObj.version.minor_ver = minor_ver;  //minor_ver = 1;版本
	IdentifierObj.serialID = serialID;            //serialID = 0x001169BC;;序列号
	IdentifierObj.product_name = product_name;    //product_name = {8, "ADC4"};产品名称
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
    	 g_DeviceNetRequstData = 0;//请求标志清0
    }
    


}

/*******************************************************************************
* 函数名:	void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参  :	struct DefFrameData * pFrame，接收报文数组
* 返回值:    	无
* 功能描述:	DeviceNet类服务函数
            DeviceNet类只有1个属性，可选执行Get_Attribute_Single服务，响应其版本信息
*******************************************************************************/
void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    
	if(pReciveFrame->pBuffer[1] != SVC_GET_ATTRIBUTE_SINGLE)        //不支持的服务
	{	
        //组2信息+源MAC ID(从站ID)
        //信息id=3，表示从站响应主站的显示请求
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);        //
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);        //目的MAC ID(主站ID)
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;  //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;    //ERR_SERVICE_NOT_SUPPORT ,不支持的服务
		pSendFrame->pBuffer[3] = ERR_RES_INAVAIL;            //ERR_RES_INAVAIL ,对象执行服务的资源不可用(附加错误代码)
		pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        //发送
		SendData(pSendFrame);                    //发送错误响应报文
		return ;
	}
	if(pReciveFrame->pBuffer[2] != 1)                               //不存在的属性
	{	
	    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
		pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;             //ERR_ID_INAVAIL ,不存在指定的类/实例/属性ID(附加错误代码)
		pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;     //ERR_NO_ADDITIONAL_DESC ,无附加描述代码
        pSendFrame->len = 4;
         pReciveFrame->complteFlag = 0;
		//发送
		SendData(pSendFrame);                    //发送错误响应报文
		return ;
	}
	//执行显示请求
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);            //目的MAC ID(主站ID)
	pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1表示响应，VC_Get_Attribute_Single服务代码
	pSendFrame->pBuffer[2] = DeviceNetClass.version;
	pSendFrame->pBuffer[3] = DeviceNetClass.version >> 8;   //类的版本信息
    pSendFrame->len = 4;
     pReciveFrame->complteFlag = 0;
	//发送
	SendData(pSendFrame);                        //发送显示信息的响应报文
}
/******************************************************************************** 
*函数名 : void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
*形  参 : struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收报文数组
*返回值 :  	无
*功能描述: DeviceNet对象服务函数
********************************************************************************/
void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
   
    switch( pReciveFrame->pBuffer[1])
    {
    case (SVC_GET_ATTRIBUTE_SINGLE):         //获取单个属性服务
	{        
        switch(pReciveFrame->pBuffer[4]) //属性ID
        {
            case DEVICENET_OBJ_MACID:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);              
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);    //目的MAC ID(主站ID)
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1表示响应，SVC_GET_ATTRIBUTE_SINGLE服务代码
                pSendFrame->pBuffer[2] = DeviceNetObj.MACID;    //源MAC ID(从站ID)
                pSendFrame->len = 3;
                pReciveFrame->complteFlag = 0;
                //发送
                SendData(pSendFrame);                //发送显示信息的响应报文
                return ;             
            }
            case  DEVICENET_OBJ_BAUD:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                pSendFrame->pBuffer[2] = DeviceNetObj.baudrate;//从站波特率
                pSendFrame->len = 3;
                pReciveFrame->complteFlag = 0;
                //发送
                SendData(pSendFrame);
                return;
            }
            case DEVICENET_OBJ_MASTERID:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.select;       //分配选择
                pSendFrame->pBuffer[3] = DeviceNetObj.assign_info.master_MACID; //主站MAC ID
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //发送
                SendData(pSendFrame);
                return ;
            }
            default:
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);       //目的MAC ID(主站ID)
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE; //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
                pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;            //ERR_ID_INAVAIL ,不存在指定的类/实例/属性ID(附加错误代码)
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;    //ERR_NO_ADDITIONAL_DESC ,无附加描述代码
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //发送
                SendData(pSendFrame);                   //发送错误响应报文
                return ;
            }        
        }  
        break;
	}
	case ( SVC_SET_ATTRIBUTE_SINGLE):      //设置单个属性服务
	{
        switch (pReciveFrame->pBuffer[3])
        {
            case DEVICENET_OBJ_MACID:
            case DEVICENET_OBJ_BAUD:
            case DEVICENET_OBJ_MASTERID://不支持设置节点地址、波特率、分配选择和主站MAC ID
            {
                pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                pSendFrame->pBuffer[2] = ERR_PROPERTY_NOT_SET;
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                pSendFrame->len = 4;
                pReciveFrame->complteFlag = 0;
                //发送
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
                //发送
                SendData(pSendFrame);
                return ;			           
            }
        }		
	}
	case (SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)://建立连接服务代码
    case (SVC_RELEASE_GROUP2_IDENTIFIER_SET):  //释放组2连接服务
	{	
		UnconVisibleMsgService(pReciveFrame, pSendFrame);                            //非连接显式信息服务，
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
		//发送
		SendData(pSendFrame);
		return ;
	}
    }
}
/*******************************************************************************
* 函数名:	void ConnectionClassService(BYTE  *buf)
* 形  参 :	BYTE  *buf，接收报文数组
* 返回值:    	无
* 功能描述:	连接类服务函数
********************************************************************************/
void ConnectionClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);               //目的MAC ID(主站ID)
	pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;         //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;           //ERR_SERVICE_NOT_SUPPORT ,不支持的服务
	pSendFrame->pBuffer[3] = 0x01;                              //附加错误代码
    pSendFrame->len = 4;
    
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);//发送
	return ;
}
/*******************************************************************************
* 函数名:	void VisibleConnectObjService(BYTE  *buf)
* 形参:	BYTE  *buf，接收报文数组
* 返回值:    	无
* 功能描述:	显式信息连接服务函数
*******************************************************************************/
void VisibleConnectObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);     // 目的MAC ID(主站ID)
    
    switch( pReciveFrame->pBuffer[1])
    {
        case SVC_SET_ATTRIBUTE_SINGLE://设置服务，#define SVC_SET_ATTRIBUTE_SINGLE	0x10
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_SET_ATTRIBUTE_SINGLE;// R/R=1表示响应，pReciveFrame->pBuffer[1]是接收报文中的服务代码
            pSendFrame->pBuffer[2] =  pReciveFrame->pBuffer[5]; //根据需要添加长度限制
            pSendFrame->pBuffer[3] =  pReciveFrame->pBuffer[6];;
            pSendFrame->len = 4;
            break;       
        }
        case SVC_GET_ATTRIBUTE_SINGLE://获取服务
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
            switch(pReciveFrame->pBuffer[4])
            {
                case 7:                   //属性ID为7，表示通过本连接发送的最大字节数为8个
                {	 			
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //属性ID为8，表示通过本连接接收的最大字节数为8个
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
	//发送
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
}
/********************************************************************************
* 函数名:	void CycInquireConnectObjService(BYTE  *buf)
* 形  参:	BYTE  *buf，接收报文数组
* 返回值:    	无
* 功能描述:	轮询信息连接实例服务函数
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
            switch (pReciveFrame->pBuffer[4]) //检查属性
            {
                case 1: //state 设置  4-超时 3-已建立好连接 1-配置状态
                {                  
                    if (pReciveFrame->pBuffer[5] <= 4) //检查是否为有效状态
                    {
                        //TODO:需要进一步检查状态--状态如何转变
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
                case 7:                   //属性ID为7，表示通过本连接发送的最大字节数为8个
                {	 			
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //属性ID为8，表示通过本连接接收的最大字节数为8个
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
* 函数名:	void IdentifierClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参:	BYTE  *buf，接收报文数组
* 返回值:    	无
* 功能描述:	标识符类服务, 不支持任何服务，错误响应
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
* 函数名:	void IdentifierObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形  参:	BYTE  *buf，接收报文数组
* 返回值:    	无
* 功能描述:	标识符对象服务函数，响应主站有关标示符的请求
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
            case IDENTIFIER_OBJ_SUPPLIERID:  //属性ID1，表示用数字标识各供应商
            {
                pSendFrame->pBuffer[2] = IdentifierObj.providerID;
			    pSendFrame->pBuffer[3] = IdentifierObj.providerID >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);                
                break;
            }
            case IDENTIFIER_OBJ_TYPE://属性ID2，表示产品通用类型说明
            {
                pSendFrame->pBuffer[2] = IdentifierObj.device_type;
			    pSendFrame->pBuffer[3] = IdentifierObj.device_type >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_CODE://属性ID3，表示产品代码
            {
                pSendFrame->pBuffer[2] = IdentifierObj.product_code;
			    pSendFrame->pBuffer[3] = IdentifierObj.product_code >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_VERSION://属性ID4，表示产品版本
            {
                pSendFrame->pBuffer[2] = IdentifierObj.version.major_ver;
			    pSendFrame->pBuffer[3] = IdentifierObj.version.minor_ver;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
             case IDENTIFIER_OBJ_STATUES://属性ID5，表示设备状态概括
             {
                 pSendFrame->pBuffer[2] = IdentifierObj.device_state;
			     pSendFrame->pBuffer[3] = IdentifierObj.device_state >> 8;
                 pSendFrame->len = 4;
			     SendData(pSendFrame);
                 break;
             }
            case IDENTIFIER_OBJ_SERIALNUM://属性ID6，表示设备序列号
            {
             	pSendFrame->pBuffer[2] = IdentifierObj.serialID;
			    pSendFrame->pBuffer[3] = IdentifierObj.serialID >> 8;
                pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 16;
                pSendFrame->pBuffer[5] = IdentifierObj.serialID >> 24;
                pSendFrame->len = 6;
                SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_NAME://属性ID7，表示产品名称
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
            default://不支持的服务
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
* 函数名:	void RountineClassObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参:	无
* 返回值:    	无
* 功能描述:	信息路由器服务，不支持任何服务，错误响应
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
* 函数名:	void InitVisibleConnectionObj(void)
* 形  参:	无
* 返回值:    	无
* 功能描述:	显式信息连接配置函数
********************************************************************************/   
void InitVisibleConnectionObj(void)
{
	VisibleConnectionObj.state = STATE_LINKED;	                //已建立状态
	VisibleConnectionObj.instance_type = 0x00;	        //显式信息连接
	VisibleConnectionObj.transportClass_trigger = 0x83;	//传输分类3
	VisibleConnectionObj.produced_connection_id = 0xFF;	
	VisibleConnectionObj.consumed_connection_id = 0xFF;	
	VisibleConnectionObj.initial_comm_characteristics = 0x22;	//通过信息组1生产,通过信息组2消费
	VisibleConnectionObj.produced_connection_size = 0xFF;	    
	VisibleConnectionObj.consumed_connection_size = 0xFF;		//最大接受FF字节
	VisibleConnectionObj.expected_packet_rate = 0x09C4;		    //必须设定期望包速率
	VisibleConnectionObj.watchdog_timeout_action = 1;		    //转为超时状态
	VisibleConnectionObj.produced_connection_path_length = 0;	
	VisibleConnectionObj.consumed_connection_path_length = 0xFF;	
	VisibleConnectionObj.produced_inhibit_time = 0;		        //无时间间隔
}
/******************************************************************************* 
* 函数名:	void InitCycleInquireConnectionObj(void)
* 返回值:    	无
* 功能描述:	I/O轮询连接配置函数
* 形参:	无
********************************************************************************/    
void InitCycleInquireConnectionObj(void)
{
	CycleInquireConnedctionObj.state = STATE_LINKED;	                //配置状态
	CycleInquireConnedctionObj.instance_type = 0x01;	        //I/O连接
	CycleInquireConnedctionObj.transportClass_trigger = 0x82;	//传输分类2
	CycleInquireConnedctionObj.produced_connection_id = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_id = 0xFF;	
	CycleInquireConnedctionObj.initial_comm_characteristics = 0x01;	//通过信息组1生产,通过信息组2消费
	CycleInquireConnedctionObj.produced_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.consumed_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.expected_packet_rate = 0;		        //设定期望包速率
	CycleInquireConnedctionObj.watchdog_timeout_action = 0;		    //转为超时状态
	CycleInquireConnedctionObj.produced_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.produced_inhibit_time = 0;		        //无时间间隔
}
/******************************************************************************* 
* 函数名:	void InitStatusChangedConnectionObj(void)
* 返回值:    	无
* 功能描述: 状态改变连接配置函数
* 形参:	无
********************************************************************************/    
void InitStatusChangedConnectionObj(void)
{
	StatusChangedConnedctionObj.state = STATE_LINKED;	                //建立连接
	StatusChangedConnedctionObj.instance_type = STATUS_CHANGE;	       
	StatusChangedConnedctionObj.transportClass_trigger = 0x82;	//传输分类2
	StatusChangedConnedctionObj.produced_connection_id = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_id = 0xFF;	
	StatusChangedConnedctionObj.initial_comm_characteristics = 0x01;	//通过信息组1生产,通过信息组2消费
	StatusChangedConnedctionObj.produced_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.consumed_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.expected_packet_rate = 0;		        //设定期望包速率
	StatusChangedConnedctionObj.watchdog_timeout_action = 0;		    //转为超时状态
	StatusChangedConnedctionObj.produced_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.produced_inhibit_time = 0;		        //无时间间隔

    LoopStatusSend.startTime = CpuTimer0.InterruptCount;
    LoopStatusSend.delayTime = 3000; //3s 上传一次状态信息
}
/*******************************************************************************  
** 函数名:	void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	    struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:    	无
** 功能描述:	CAN信息过滤器函数，提取帧ID1和帧ID2中的信息，
                仅限组2设备，并对信息进行分类处理
*******************************************************************************/
void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
    BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
	
	if(mac == DeviceNetObj.MACID)  //仅限组2设备
	{	        
        switch( function)
        {
            case GROUP2_REPEAT_MACID:  //重复MAC ID检查信息
            {          
                if (mac == DeviceNetObj.MACID)
                {
                    ResponseMACID(pReciveFrame, 0x80);       //重复MACID检查响应函数,应答，物理端口为0
                }
                return; //MACID不匹配,丢弃
            }
            case GROUP2_VSILBLE_ONLY2: //1100 0000：非连接显示请求信息，预定义主从连接
            {                
				UnconVisibleMsgService(pReciveFrame, pSendFrame);    //非连接显式信息服务
                return;
            }
            case  GROUP2_POLL_STATUS_CYCLE: //主站I/O轮询命令/状态变化/循环信息
            {     
                CycleInquireMsgService(pReciveFrame, pSendFrame);     // I/O轮询信息服务
                return ;    
            }
            case GROUP2_VSILBLE:  //主站显式请求信息
            {               
				VisibleMsgService(pReciveFrame, pSendFrame);        //显式信息服务
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
* 函数名:	void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
* 形参:	无
* 返回值:    	无
* 功能描述:	检查重复MACID响应函数
******************************************************************************/
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
{                        //重复MACID检查
    pSendFrame->ID =  MAKE_GROUP2_ID( GROUP2_REPEAT_MACID, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = config;	                        //请求/响应标志=1，表示响应，端口号0
	pSendFrame->pBuffer[1]= IdentifierObj.providerID;	//制造商ID低字节
	pSendFrame->pBuffer[2] = IdentifierObj.providerID >> 8;	//制造商ID高字节
	pSendFrame->pBuffer[3] = IdentifierObj.serialID;	                    //序列号低字节
	pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 8;                //序列号中间字节1
	pSendFrame->pBuffer[5] = IdentifierObj.serialID >>16;          //序列号中间字节2
	pSendFrame->pBuffer[6] = IdentifierObj.serialID >>24;	//序列号高字节
    pSendFrame->len = 7;
	SendData(pSendFrame);                      //发送报文
}
/*******************************************************************************
* 函数名:	BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 功能描述:	主动检查重复MACID函数。
* 形参:	无
* 返回值:       TRUE    网络上有和自己重复的地址
                FALSE   网络上没有和自己重复的地址
*******************************************************************************/
BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{	
    int sendCount = 0; 
    do
    {
        pReciveFrame->complteFlag = 0;
           //发送请求
        ResponseMACID( pSendFrame, 0);
    	if (g_CANErrorStatus != 0) //通讯错误
    	{
    		return TRUE;
    	}
        StartOverTimer();//启动超时定时器
        while( IsTimeRemain())
        {
            if ( pReciveFrame->complteFlag)//判断是否有未发送的数据
            {
                BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
                BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
                if (function == GROUP2_REPEAT_MACID)
                {                  
                    if (mac == DeviceNetObj.MACID)
                    {
                         pReciveFrame->complteFlag = 0;
                          return TRUE; //只要有MACID一致，无论应答还是发出，均认为重复
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
	return FALSE;	//没有重复地址
}
/********************************************************************************
** 函数名:	void CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      BYTE 0-溢出 非零为检测通过
** 功能描述:j检测非连接显式信息服务设置连接代码
********************************************************************************/
BOOL CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //错误
    BYTE errorAdd = 0; //附加错误描述
    
    //如果已分配主站,则检查是否来自同一主站
    if((IdentifierObj.device_state & 0x01) && (pReciveFrame->pBuffer[5] != DeviceNetObj.assign_info.master_MACID))	//验证主站
    {	//不是来自当前主站，错误响应
        error =  ERR_OBJECT_STATE_INFLICT;
        errorAdd = 0x01;     
    }
    //仅限组2无连接信息，这个报文指向DeviceNet对象，其类ID为3。在每个DeviceNet的物理连接中只有一个DeviceNet类的实例，因此实例ID为1
    else if(pReciveFrame->pBuffer[2] != 3 || pReciveFrame->pBuffer[3] != 1)	//验证类ID和实例ID
    {   //验证类ID和实例ID错误，错误响应
        error =   ERR_PROPERTY_VALUE_INAVAIL;
        errorAdd = ERR_NO_ADDITIONAL_DESC;         
    }
    else if(pReciveFrame->pBuffer[4] == 0)	//验证分配选择字节
    {//配置字节为零，主站没有配置从站，错误响应
        error =  ERR_PROPERTY_VALUE_INAVAIL; //(0x80 | ERR_PROPERTY_VALUE_INAVAIL)?
        errorAdd = 0x02;    
    }
    else if(pReciveFrame->pBuffer[4] & ~(CYC_INQUIRE | VISIBLE_MSG | BIT_STROKE|STATUS_CHANGE ))  
    {//如果不是轮询配置、显示连接、位选通，错误响应
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
        SendData(pSendFrame);  //发送报文
        return FALSE;
    }
    return TRUE;
}
    
/********************************************************************************
** 函数名:	void CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      BYTE 0-溢出 非零为检测通过
** 功能描述:检测非连接显式信息服务释放连接代码
********************************************************************************/
BOOL CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //错误
    BYTE errorAdd = 0; //附加错误描述
    USINT config = pReciveFrame->pBuffer[4];
    if(config == 0)   //如果配置字节为0
    {	
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);      
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);
        return FALSE;
    }
    if(config & ~(CYC_INQUIRE | VISIBLE_MSG |BIT_STROKE|STATUS_CHANGE))//不支持的连接，错误响应
    {
        error = ERR_RES_INAVAIL;
        errorAdd = 0x02;
    }
    else if((config & DeviceNetObj.assign_info.select) == 0)//连接不存在，错误响应
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
        SendData(pSendFrame);  //发送报文
        return FALSE;
    }
    return TRUE;
}


/********************************************************************************
** 函数名:	void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      无
** 功能描述:	非连接显式信息服务函数，主站用该报文命令从站配置连接
********************************************************************************/
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{ 
    if(pReciveFrame->pBuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pBuffer[1]是收到的服务代码
	{
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //检测未通过返回
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pBuffer[5];  //主站告诉从站：主站的地址
        USINT config = pReciveFrame->pBuffer[4];
		DeviceNetObj.assign_info.select |= config;       //配置字节
        

		if(config & CYC_INQUIRE)                          //分配I/O轮询连接
		{	
			InitCycleInquireConnectionObj();                       //I/O轮询连接配置函数
			CycleInquireConnedctionObj.produced_connection_id = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			CycleInquireConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
 	        //成功执行响应
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // 目的MAC ID(主站ID)
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
			SendData(pSendFrame);             //发送报文
			return ;
		}
		if(config & VISIBLE_MSG)
		{	
			InitVisibleConnectionObj();//分配显式信息连接
			VisibleConnectionObj.produced_connection_id =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
			VisibleConnectionObj.consumed_connection_id =  MAKE_GROUP2_ID(GROUP2_VSILBLE, DeviceNetObj.MACID);
			//成功执行响应
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	//信息体格式0,8/8：类Class ID = 8 位整数，实例Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
			//发送
			SendData(pSendFrame);			
		}
      if(config & STATUS_CHANGE)                          //分配主从
		{	
			InitStatusChangedConnectionObj();                       //状态改变连接配置函数
			StatusChangedConnedctionObj.produced_connection_id = MAKE_GROUP1_ID( GROUP1_STATUS_CYCLE_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			StatusChangedConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
 	        //成功执行响应
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // 目的MAC ID(主站ID)
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
			SendData(pSendFrame);             //发送报文

			return ;
		}
		if(config &  BIT_STROKE) //分配位选通连接
		{	
		
//			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
//			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
//			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
//			pSendFrame->pBuffer[2] = 0;	//信息体格式0,8/8
//            pSendFrame->len = 3;
//			SendData(pSendFrame);
			return ;
		}
		IdentifierObj.device_state |= 0x01;	//设备已和主站连接
		return ;
	}
	else if(pReciveFrame->pBuffer[1] == SVC_RELEASE_GROUP2_IDENTIFIER_SET)   //释放连接服务
	{
		if(!CheckReleaseCode(pReciveFrame, pSendFrame))
        {
            return;
        }
		//释放连接
        USINT config = pReciveFrame->pBuffer[4];
        
		DeviceNetObj.assign_info.select |= (config^0xff); //取反释放相应的连接
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
		//执行成功响应
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
		pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
		pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);		
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
		SendData(pSendFrame);
	}
	else
	{	//组2非连接显式信息服务不支持，错误响应
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
** 函数名:	void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      无
** 功能描述:	显式信息服务函数，执行主站的显示请求响应
*********************************************************************************/
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	BYTE class, obj;

	class = pReciveFrame->pBuffer[2]; //类ID
	obj = pReciveFrame->pBuffer[3];   //实例ID

	//信息路由
	if(!(DeviceNetObj.assign_info.select & VISIBLE_MSG))	//没有建立显式信息连接
		return ;
    
    switch(class)
    {
        case 0x01: //标识符对象
        {   
            if(obj == 0)	    //类服务
            {
                IdentifierClassService( pReciveFrame, pSendFrame);
            }
            else if(obj == 1)	//实例1服务
            {
                IdentifierObjService(pReciveFrame, pSendFrame);
            }              
            break;
        }
        case 0x02: //信息路由器对象
        {
            RountineClassObjService(pReciveFrame, pSendFrame);
            break;
        }
        case 0x03://DeviceNet对象
        {
            if(obj == 0)	    //类服务
            {
                DeviceNetClassService(pReciveFrame, pSendFrame);
                return ;
            }
            else if(obj == 1)	//实例1服务
            {
                DeviceNetObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }
        case 0x05:	//连接对象
        {
            if(obj == 0)	    //类服务
            {
                ConnectionClassService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 1)	//显式信息连接
            {
                VisibleConnectObjService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 2)	//I/O轮询连接
            {
                CycInquireConnectObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }   
    } 
}
/********************************************************************************
** 函数名:	void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 功能描述:	I/O轮询信息服务函数，在主站和从站之间传输数据
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      无
*********************************************************************************/
static void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{


    if(CycleInquireConnedctionObj.state != STATE_LINKED )	//轮询I/O连接没建立
		return ;

    g_DeviceNetRequstData |= 0x0003; //置位请求标志
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
 * 应答循环请求服务
 * @brief   应答循环帧处理
 */
static void AckCycleInquireMsgService(void)
{
	uint8_t result = 0;
	//不处理完整，不接收新的帧
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
 * 对发送IO数据进行打包，使用轮询
 * @brief   应答循环帧处理
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
 * 对发送IO数据进行打包,使用状态变化信息
 * @brief   应答循环帧处理
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
** 函数名:	void DeviceMonitorPluse(void)
** 功能描述:	设备监测脉冲函数
** 形参:	无
** 返回值:      无
********************************************************************************/
void DeviceMonitorPluse(void)
{
//	if(query_time_event(2))
//	{
//		start_time(160);
//		//发送检测脉冲
//		pSendFrame->pBuffer[0] = 0x80 | DeviceNetObj.MACID;
//		pSendFrame->pBuffer[1] = 0x60;
//		pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
//		pSendFrame->pBuffer[3]= 0x80 | SVC_MONITOR_PLUSE;
//		pSendFrame->pBuffer[4] = 0x01;		//标识符对象ID=1
//		pSendFrame->pBuffer[5] = 0x00;
//		*(send_buf + 6) = IdentifierObj.device_state;
//		*(send_buf + 7) = 0;
//		*(send_buf + 8) = 0;
//		*(send_buf + 9) = 0;
//		SendData(10, send_buf);
//	}
}


/*******************************************************************************
* 函数名:	void DeviceNetReciveCenter(uint16* id, uint8 * pdata)
* 功能描述: 从缓冲中获取数据并解析
* 形  参: uint16* pID  11bitID标识, uint8 * pbuff 缓冲数据, uint8 len 数据长度
* 返回值:      BYTE  0-信息未进行处理  1-信息进行处理
********************************************************************************/
BOOL DeviceNetReciveCenter(UINT* pID, USINT * pbuff,USINT len)
{   
    BYTE i= 0;
    //判断是否为仅限组2---可以在滤波器设置屏蔽
    if( ((*pID) & 0x0600) != 0x0400)  //不是仅限组2报文处理
	{       
        return FALSE;    
    }        
    
    if( DeviceNetReciveFrame.complteFlag) //
    {
        return FALSE;
    }
   
    if (len <= 8) //最大长度限制
    {
         DeviceNetReciveFrame.ID = *pID;   
         DeviceNetReciveFrame.len = len;
        for(i = 0; i< len; i++) //复制数据
        {
            DeviceNetReciveFrame.pBuffer[i] = pbuff[i];
        }
        DeviceNetReciveFrame.complteFlag = 0xff;
         
         
        switch(WorkMode)
        {
            case MODE_NORMAL: //正常工作模式
            {
                CANFrameFilter(&DeviceNetReciveFrame, &DeviceNetSendFrame);
                break;
            }        
        }
         DeviceNetReciveFrame.complteFlag = 0;// 默认此处处理完成
         return TRUE;
    }
    
    
    
    
    return 0;
}
/*******************************************************************************
* 函数名:	void  SendData(uint16* id, uint8 * pdata)----根据具体平台需要重新
* 功能描述: 从缓冲中获取数据并解析
* 形  参: struct DefFrameData* pFrame
* 返回值:      null
********************************************************************************/
void SendData(struct DefFrameData* pFrame)
{
     CANSendData(pFrame->ID, pFrame->pBuffer, pFrame->len);
      pFrame->complteFlag = 0;
}
/*******************************************************************************
* 函数名:	void  StartOverTimer()----根据具体平台需要重新
* 功能描述: 启动超时定时器
* 形  参:   null
* 返回值:   null
********************************************************************************/

void StartOverTimer()
{
	StartTime = CpuTimer0.InterruptCount;
}
/*******************************************************************************
* 函数名:	BOOL IsTimeRemain()----根据具体平台需要重新
* 功能描述: 启动超时定时器
* 形  参:   null
* 返回值:   TRUE-没有超时 FALSE-超时
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
 * 应答请求服务--非紧急情况下
 * @brief   应答循环帧处理
 */
void AckMsgService(void)
{
	if (WorkMode== MODE_FAULT) //
	{
		ON_LED3;
		//TODO: 添加通讯错误处理程序，超时后复位。
		return;
	}
	//已经建立后状态改变连接---周期性报告状态/或者突发报告
	if (StatusChangedConnedctionObj.state == STATE_LINKED)
	{
		if(IsOverTime(LoopStatusSend.startTime, LoopStatusSend.delayTime) )
		{
			DeviceNetSendFrame.pBuffer[0] = 0x1A | 0x80;
			DeviceNetSendFrame.pBuffer[1] = 0xA1; //测试
			DeviceNetSendFrame.pBuffer[2] = 0xA2; //测试
			DeviceNetSendFrame.pBuffer[3] = 0xA3; //测试
			DeviceNetSendFrame.pBuffer[4] = 0xA4; //测试
			DeviceNetSendFrame.pBuffer[5] = 0xA5; //测试
			DeviceNetSendFrame.len = 6;
			PacktIOMessageStatus(&DeviceNetSendFrame);
			LoopStatusSend.startTime =  CpuTimer0.InterruptCount; //重新设置新的延时
		}


	}

	if(g_DeviceNetRequstData == 0)
	{
		return;
	}
	if ((g_DeviceNetRequstData & 0x0003)==0x0003)//轮询消息
	{
		AckCycleInquireMsgService();
		g_DeviceNetRequstData &= 0xFFFC; //清除标志位
	}




}
