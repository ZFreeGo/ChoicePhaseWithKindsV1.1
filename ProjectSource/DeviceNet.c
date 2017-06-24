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
uint16_t  providerID = 0X1234;               // 供应商ID
uint16_t  device_type = 0;                   // 通用设备
uint16_t  product_code = 0X00d2;             // 产品代码
uint8_t  major_ver = 0X01;
uint8_t  minor_ver = 0X01;                 // 版本
uint32_t  serialID = 0x001169BC;            // 序列号
SHORT_STRING  product_name = {8, (unsigned char *)"YongCi"};// 产品名称
////////////////////////////////////////////////////////////////////////////////


//////////////////////函数申明/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config);

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

uint8_t  SendBufferData[10];//接收缓冲数据
uint8_t  ReciveBufferData[10];//接收缓冲数据
struct DefFrameData  DeviceNetReciveFrame; //接收帧处理
struct DefFrameData  DeviceNetSendFrame; //接收帧处理

static volatile uint8_t WorkMode = 0; //

static volatile uint8_t StartTime = 0;


static RunTimeStamp LoopStatusSend;//循环状态发送
static RunTimeStamp OffLine;// 处于离线状态时超时复位


/*******************************************************************************
* 函数名:	void InitDeviceNet()
* 形参  :	null
* 返回值:    null
* 功能描述:	初始化DeviceNet所涉及的基本数据
*******************************************************************************/
void InitDeviceNet()
{    
	ServiceDog();
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
    	 g_DeviceNetRequstData = 0;//请求标志清0
    }
    


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
    uint8_t mac = GET_GROUP2_MAC(pReciveFrame->ID);
    uint8_t function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
    ServiceDog();
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
            	ServiceDog();
				UnconVisibleMsgService(pReciveFrame, pSendFrame);    //非连接显式信息服务
                return;
            }
            case  GROUP2_POLL_STATUS_CYCLE: //主站I/O轮询命令/状态变化/循环信息
            {     
            	ServiceDog();
                CycleInquireMsgService(pReciveFrame, pSendFrame);     // I/O轮询信息服务
                return ;    
            }
            case GROUP2_VSILBLE:  //主站显式请求信息
            {               
				//VisibleMsgService(pReciveFrame, pSendFrame);        //显式信息服务
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
* 函数名:	void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config)
* 形参:	无
* 返回值:    	无
* 功能描述:	检查重复MACID响应函数
******************************************************************************/
void ResponseMACID(struct DefFrameData* pSendFrame, uint8_t config)
{                        //重复MACID检查
	 ServiceDog();
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
    	ServiceDog();
        pReciveFrame->complteFlag = 0;
           //发送请求
        ResponseMACID( pSendFrame, 0);
    	if (g_CANErrorStatus != 0) //通讯错误
    	{
    		return TRUE;
    	}
        StartOverTimer();//启动超时定时器
        while(IsTimeRemain())
        {
        	ServiceDog();
            if ( pReciveFrame->complteFlag)//判断是否有未发送的数据
            {
                uint8_t mac = GET_GROUP2_MAC(pReciveFrame->ID);
                uint8_t function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
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
** 返回值:      uint8_t 0-溢出 非零为检测通过
** 功能描述:j检测非连接显式信息服务设置连接代码
********************************************************************************/
BOOL CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    uint8_t error = 0; //错误
    uint8_t errorAdd = 0; //附加错误描述
    ServiceDog();
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
** 返回值:      uint8_t 0-溢出 非零为检测通过
** 功能描述:检测非连接显式信息服务释放连接代码
********************************************************************************/
BOOL CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    uint8_t error = 0; //错误
    uint8_t errorAdd = 0; //附加错误描述
    uint8_t config = pReciveFrame->pBuffer[4];
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
	ServiceDog();
    if(pReciveFrame->pBuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pBuffer[1]是收到的服务代码
	{
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //检测未通过返回
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pBuffer[5];  //主站告诉从站：主站的地址
        uint8_t config = pReciveFrame->pBuffer[4];
		DeviceNetObj.assign_info.select |= config;       //配置字节
        
		ServiceDog();
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
		ServiceDog();
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
		ServiceDog();
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
        uint8_t config = pReciveFrame->pBuffer[4];
        
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

/********************************************************************************
** 函数名:	void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 功能描述:	I/O轮询信息服务函数，在主站和从站之间传输数据
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组
** 返回值:      无
*********************************************************************************/
static void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	ServiceDog();
    if(CycleInquireConnedctionObj.state != STATE_LINKED )	//轮询I/O连接没建立
		return ;
    g_DeviceNetRequstData |= 0x0003; //置位请求标志
	return ;

}

/**
 * 应答循环请求服务
 * @brief   应答循环帧处理
 */
static void AckCycleInquireMsgService(void)
{

	ServiceDog();
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
	ServiceDog();
	PacktIOMessage(&DeviceNetSendFrame);
}
/**
 * 对发送IO数据进行打包，使用轮询
 * @brief   应答循环帧处理
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
 * 对发送IO数据进行打包,使用状态变化信息
 * @brief   应答循环帧处理
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
* 返回值:      uint8_t  0-信息未进行处理  1-信息进行处理
********************************************************************************/
BOOL DeviceNetReciveCenter(uint16_t* pID, uint8_t * pbuff,uint8_t len)
{   
    uint8_t i= 0;
    //判断是否为仅限组2---可以在滤波器设置屏蔽
    ServiceDog();
    if( ((*pID) & 0x0600) != 0x0400)  //不是仅限组2报文处理
	{

    	SynCloseWaitAck(pID, pbuff, len);
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
         
        ServiceDog();
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
 * 通讯闪烁计数
 */
uint32_t flashComCn = 0;

/**
 * 应答请求服务--非紧急情况下
 * @brief   应答循环帧处理
 */
void AckMsgService(void)
{
	ServiceDog();
	if (WorkMode == MODE_FAULT) //
	{
		ON_LED3;
		//TODO: 添加通讯错误处理程序，超时后复位。
		if (IsOverTime(OffLine.startTime, OffLine.delayTime))
		{
			//重新重新进行初始化
			InitStandardCAN(0, 0);
			InitDeviceNet();
		}
		return;
	}
	//有错误
	if (  g_CANErrorStatus != 0)
	{
		ON_LED3;
		WorkMode =  MODE_FAULT; //置为故障状态
		OffLine.startTime =  CpuTimer0.InterruptCount; //重新设置新的延时
		OffLine.delayTime = 5000;

	}
	ServiceDog();
	//正常通讯指示灯
	if (flashComCn++ >200000)
	{
		TOGGLE_LED3;
		flashComCn = 0;
	}
	//已经建立后状态改变连接---周期性报告状态/或者突发报告--非预制状态
	if ((StatusChangedConnedctionObj.state == STATE_LINKED) && ( g_SynCommandMessage.synActionFlag == 0))
	{
		if(IsOverTime(LoopStatusSend.startTime, LoopStatusSend.delayTime) )
		{

			DeviceNetSendFrame.pBuffer[0] = 0x1A | 0x80;
			DeviceNetSendFrame.pBuffer[1] = g_WorkMode;  //工作模式
			DeviceNetSendFrame.pBuffer[2] = CheckVoltageStatus();//电压越限制检测
			DeviceNetSendFrame.pBuffer[3] = CheckFrequencyStatus();
			DeviceNetSendFrame.len = 4;
			PacktIOMessageStatus(&DeviceNetSendFrame);
			LoopStatusSend.startTime =  CpuTimer0.InterruptCount; //重新设置新的延时
		}


	}
	//同步合闸预制--等待应答/等待执行
	if( (g_SynCommandMessage.synActionFlag == SYN_HE_READY) ||
			(g_SynCommandMessage.synActionFlag == SYN_HE_WAIT_ACTION))
	{
		//检测是否超时
		if (IsOverTime(g_SynCommandMessage.closeWaitAckTime.startTime, g_SynCommandMessage.closeWaitAckTime.delayTime))
		{
			g_SynCommandMessage.synActionFlag = 0;
			g_PhaseActionRad[0].readyFlag = 0;
			g_PhaseActionRad[1].readyFlag = 0;
			g_PhaseActionRad[2].readyFlag = 0;

		}
	 }

	//是否有请求数据
	if(g_DeviceNetRequstData == 0)
	{
		return;
	}
	if ((g_DeviceNetRequstData & 0x0003)==0x0003)//轮询消息
	{
		ServiceDog();
		AckCycleInquireMsgService();
		g_DeviceNetRequstData &= 0xFFFC; //清除标志位
	}





}
