
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __HEADER_TEMPLATE_H
#define	__HEADER_TEMPLATE_H

#include "StdType.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#define STATUS_CHANGE  0x10    //状态改变
#define BIT_STROKE  0x04    //位选通
#define CYC_INQUIRE	0x02    //定义CYC_INQUIRE，IO轮询
#define VISIBLE_MSG	0x01    //显式信息连接

/////////////数据类型///////////////////////
typedef uint8_t BOOL;         //1BYTE
typedef int8_t SINT;          //1BYTE
typedef int16_t INT;          //2BYTE

typedef struct{int32_t a; int32_t b;} LINT;

typedef struct{uint32_t a; uint32_t b;} ULINT;
typedef float32_t  REAL;              //4BYTE
typedef float32_t  LREAL;             //4BYTE
typedef INT ITIME;
typedef int32_t TIME;
typedef int32_t FTIME;
typedef LINT LTIME;
typedef uint16_t DATE;

typedef struct{uint16_t length; uint8_t *ucdata;} STRING;
typedef struct{uint16_t length; int16_t *undata;} STRING2;
typedef	struct{uint16_t length; uint8_t *ucdata;} STRINGN;
typedef struct{uint16_t length; uint8_t *ucdata;} SHORT_STRING;

typedef struct{ uint32_t a;  uint32_t b;} LWORD;
typedef struct{uint8_t type; uint8_t value;} EPATH;
typedef uint16_t ENGUNITS;
///////////////服务代码定义/////////////////////////
#define SVC_Get_Attributes_All	0x01
#define SVC_Set_Attributes_All	0x02
#define SVC_Get_Attribute_List	0x03
#define SVC_Set_Attribute_List	0x04
#define SVC_RESET				0x05
#define SVC_START				0x06
#define SVC_STOP				0x07
#define SVC_CREATE				0x08
#define SVC_DELETE				0x09
#define SVC_Apply_Attributes	0x0D
#define SVC_GET_ATTRIBUTE_SINGLE	0x0E
#define SVC_SET_ATTRIBUTE_SINGLE	0x10
#define SVC_Find_Next_Object_Instance	0x11
#define SVC_ERROR_RESPONSE		0x14
#define SVC_RESTORE				0x15
#define SVC_SAVE				0x16
#define SVC_NOP					0x17
#define SVC_Get_Member			0x18
#define SVC_Set_Member			0x19
#define SVC_Insert_Member		0x1A
#define SVC_Remove_Member		0x1B
#define SVC_MONITOR_PLUSE		0x4D
#define SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET	0x4B
#define SVC_RELEASE_GROUP2_IDENTIFIER_SET			0x4C

////////////////////错误描述///////////////////////////////////////
#define ERR_SUCCESS 				0x00	//成功执行了服务
#define ERR_RES_INAVAIL 			0x02	//对象执行服务的资源不可用
#define ERR_PATH_ERR 				0x04	//不理解路径段标识符或句法
#define ERR_PATH_AMBIGUITY 			0x05	//路径错误
#define ERR_CONNECTION_LOSE 		0x07	//消息连接丢失
#define ERR_SERVICE_NOT_SUPPORT 	0x08	//不支持的服务
#define ERR_PROPERTY_VALUE_INAVAIL 	0x09	//无效属性值
#define ERR_PROPERTY_LIST_ERR 		0x0A	//属性列表错误
#define ERR_EXISTED_MODE 			0x0B	//已经处于请求的模式/状态
#define ERR_OBJECT_STATE_INFLICT 	0x0C	//对象状态冲突
#define ERR_OBJECT_EXISTING 		0x0D	//对象已存在
#define ERR_PROPERTY_NOT_SET 		0x0E	//属性不可设置
#define ERR_RIGHT_OFFEND 			0x0F	//权利违反
#define ERR_DEVICE_CONFLICT 		0x10	//设备当前模式禁止执行请求的服务
#define ERR_RES_DATA_TOO_LARGE 		0x11	//准备传送的数据大于分配的缓冲区
#define ERR_LACK_OF_DATA 			0x13	//执行指定操作所需的数据不够
#define ERR_PROPERTY_NOT_SUPPORT 	0x14	//不支持请求中指定的数据
#define ERR_DATA_TOO_MUCH 			0x15	//服务提供的数据太多
#define ERR_OBJECT_NOT_EXISTING 	0x16	//不存在指定的设备
#define ERR_PROPERTY_NOT_SAVE 		0x18	//请求服务前没有保存对象的属性数据
#define ERR_PROPERTY_SAVE_FAILED 	0x19	//由于尝试失败，没有保存对象的属性数据
#define ERR_PROPERTY_LIST_LOSE 		0x1C	//缺少执行服务必须的属性
#define ERR_PROPERTY_LIST_INAVAIL 	0x1D	//服务返回附带无效属性状态信息的属性列表
#define ERR_PROVIDER_ERR 			0x1F	//设备供应商规定错误
#define ERR_PROPERTY_INAVAIL 		0x20	//与请求相关的参数无效
#define ERR_PROPERTY_NO_SET 		0x27	//试图设置一个此时无法设置的属性
#define ERR_ID_INAVAIL 				0x28	//在指定的类/实例/属性中不存在指定的ID
#define ERR_ID_NOT_SET 				0x29	//无法设置请求的成员
#define ERR_GROUP2_ERR 				0x2A	//仅由组2服务器报告，且仅在服务不支持，属性不支持，或属性不可设置时

#define ERR_NO_ADDITIONAL_DESC		0xFF	//无附加描述

//State状态属性
#define STATE_NOT_EXIST     0
#define STATE_CONFIG        1
#define STATE_WAIT_LINK_ID  2
#define STATE_LINKED        3 
#define STATE_WAIT_TIME     4
#define STATE_DELAY_DELETE  5



/////////标示符结构体/////////////////////////////////////
struct DefIdentifierObject
{
	uint16_t providerID;	//供应商ID=1
	uint16_t device_type;	//设备类型ID=2
	uint16_t product_code;	//产品代码ID=3
	struct{				//版本ID=4
		uint8_t major_ver;
		uint8_t minor_ver;
	}version;
	uint32_t device_state;	//设备状态ID=5
	uint32_t serialID;		//序列号ID=6
	SHORT_STRING product_name;	//产品名称ID=7
};
/////////对象类结构体/////////////////////////////////////
struct DefDeviceNetClass
{
	uint16_t version;	//ID=1,分类定义修正版本，当前为2
};
/////////DeviceNet对象结构体/////////////////////////////////////
struct DefDeviceNetObj
{
	uint8_t MACID;	//ID=1,节点地址，缺省值63
	uint8_t baudrate;	//ID=2,波特率，缺省值125k
	struct {uint8_t select; uint8_t master_MACID;}assign_info;	//ID=5,分配信息。支持预定义主从连接，则必须支持该属性。master_MACID缺省值为255
};
/////////连接结构体/////////////////////////////////////
struct DefConnectionObj
{
	uint8_t state;					//ID=1,对象状态
	uint8_t instance_type;			//ID=2,区分I/O和显式信息连接
	uint8_t transportClass_trigger;	//ID=3,定义连接行为
	uint16_t produced_connection_id;	//ID=4,发送时,放置在CAN标识区中
	uint16_t consumed_connection_id;	//ID=5,CAN标识区中的值,指示要接受的数据
	uint8_t initial_comm_characteristics;	//ID=6,定义信息组
	uint16_t produced_connection_size;	//ID=7,最大发送字节数
	uint16_t consumed_connection_size;	//ID=8,最大接受字节数
	uint16_t expected_packet_rate;		//ID=9,与连接有关的定时
	uint8_t watchdog_timeout_action;	//ID=12,定义如何处理休眠/看门狗超时
	uint16_t produced_connection_path_length;	//ID=13,produced_connection_path字节数
	EPATH produced_connection_path[6];	//ID=14,指定通过该连接生成数据的应用对象
	uint16_t consumed_connection_path_length;	//ID=15,consumed_connection_path字节数
	EPATH consumed_connection_path[6];	//ID=16,指定通过该连接消费数据的应用对象
	uint16_t produced_inhibit_time;		//ID=17,定义产生新数据的最小间隔
};

//DeviceNet对象
#define DEVICENET_OBJ_MACID    1 //DeviceNet 对象节点地址
#define DEVICENET_OBJ_BAUD   2 //DeviceNet 对象波特率
#define DEVICENET_OBJ_MASTERID   5 //DeviceNet 对象分配选择和主站MAC ID

//标识对象
#define IDENTIFIER_OBJ_SUPPLIERID 1 //供应商ID
#define IDENTIFIER_OBJ_TYPE       2 //产品类型
#define IDENTIFIER_OBJ_CODE       3 //产品代码
#define IDENTIFIER_OBJ_VERSION    4 //版本
#define IDENTIFIER_OBJ_STATUES    5 //状态
#define IDENTIFIER_OBJ_SERIALNUM  6 //序列号
#define IDENTIFIER_OBJ_NAME       7 //产品名称

//Group1 功能码
#define  GROUP1_STATUS_CYCLE_ACK           13///从站IO状态改变或循环报文
#define  GROUP1_BIT_STROKE                 14///从站IO位选通应答报文
#define  GROUP1_POLL_STATUS_CYCLER_ACK     15///从站IO轮询或状态变化/循环应答消息


//Group2 功能码
#define GROUP2_BIT_STROBE     0 //主站IO位选通响应消息
#define GROUP2_POLL           1  //主站IO多点响应消息响应消息
#define GROUP2_STATUS_POLL        2  //主站状态变换或循环应答报文应答消息
#define GROUP2_VISIBLE_UCN    3  //从站站显示响应消息

#define GROUP2_VSILBLE        4 //主站显示请求信息
#define GROUP2_POLL_STATUS_CYCLE         5//主站IO轮询/状态变化/循环报文
#define GROUP2_VSILBLE_ONLY2  6//仅限组2非连接显示请求信息
#define GROUP2_REPEAT_MACID   7//重复MACID 检查消息

//定义完整接收数据结构
struct DefFrameData
{
      int32_t ID;     //11bitID标识
      uint8_t len;    //数据长度
      uint8_t* pBuffer; //缓冲数据
      volatile uint8_t complteFlag; //处理完成标志 非0--未处理完成；0--处理已经完成，可以重复使用
};

#define GET_GROUP1_MAC(id)   ( (uint8_t)(id)&0x003F )

#define GET_GROUP_NUM(id)  ((((id) >> 9))&0x0003)
//获取仅组2MAC地址 id为16bit
#define GET_GROUP2_MAC(id)   ( (((id) >> 3))&0x003F ) 
#define GET_GROUP2_FUNCTION(id)  ((id)&0x0007 ) 
#define GROUP2_MSG 2 //组2报文
//生成GROUP1 ID
#define MAKE_GROUP1_ID(function, mac_id) (int32_t)((((uint16_t)(function) &0x1F)<<6) | ((uint16_t)(mac_id) & 0x3F))
//生成GROUP2 ID
#define MAKE_GROUP2_ID(function, mac_id)  (int32_t)( (0x0400) | ((int32_t)((mac_id) &0x3F)<<3) | ((uint16_t)(function) & 0x07))



/**
 *运行时间戳
 */
typedef struct TagRunTimeStamp
{
    uint32_t startTime; //启动时间
    uint32_t delayTime; // 延时时间
}RunTimeStamp;

//////////////供其他模块调用的函数///////////////
extern void CANFrameFilter(struct DefFrameData * pReciveBuf, struct DefFrameData * pSendBuf);
extern unsigned char CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
extern void DeviceMonitorPluse(void);
extern BOOL DeviceNetReciveCenter(uint16_t* pID, uint8_t * pbuff,uint8_t len);
extern void InitDeviceNet();

extern void AckMsgService(void);
extern void PacktIOMessage( struct DefFrameData* pSendFrame);
//////////////供其他模块调用的变量/////////////////
extern struct DefDeviceNetObj  DeviceNetObj;
extern struct DefIdentifierObject   IdentifierObj;
extern struct DefConnectionObj  VisibleConnectionObj;
extern struct DefConnectionObj CycleInquireConnedctionObj;



#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

