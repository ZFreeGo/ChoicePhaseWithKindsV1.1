/*
 * StdTypedef.h
 *
 *  Created on: 2017年4月1日
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_STDTYPE_H_
#define PROJECTHEADER_STDTYPE_H_

typedef short           int8_t;
typedef short           int16_t;
typedef long            int32_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned long   uint32_t;
typedef float           float32_t;
typedef long double     float64_t;

#define UINT8_MAX                   255
#define UINT16_MAX                65535
#define UINT32_MAX           4294967295u

#define FALSE 0
#define TRUE 0xFF


/**
 *数据指针结构
 */

typedef struct TagPointUint8
{
    uint8_t* pData; //数据指针
    uint8_t len; //指向数据长度
}PointUint8;

/**
 *动作相角与弧度归一化值
 */
typedef struct TagActionRad
{
	uint8_t enable; //使能标志，0-禁止，非零使能
    uint8_t phase; //相A-1,B-2,C-3
    uint16_t  actionRad; //设定弧度归一化值r = M/65536 *2*PI

}ActionRad;


#endif /* PROJECTHEADER_STDTYPE_H_ */
