#ifndef __CHENJUN_MSG_H__
#define __CHENJUN_MSG_H__

#include "common.h"
#include "can.h"
#include "rtc.h"
#include "timer.h"
#include "time_task.h"
#include "ccp_manager.h"


// 发送消息到4G
#define MSG_TO_4G_CAN					0x01	// CAN消息类型
#define MSG_TO_4G_UPDATE_MCU			0x20
#define MSG_TO_4G_UPDATE_4G				0x21	

// 从4G接收消息
#define MSG_FROM_4G_CAN					0x90	// CAN消息类型
#define MSG_FROM_4G_UPDATE_MCU			0xA0
#define MSG_FROM_4G_UPDATE_4G			0xA1

#define MSG_CAN_BRIDGE_TIMEOUT			10000	// 10000 ms = 10s 未接收到4G的数据则停止发送can数据


void MSG_Init(void);

/**
 * 功能：	处理can报文数据
 * 参数：	空
 * 返回：	空
 */
void MSG_DealCanMsg(void);

/**
 * 功能：	处理从4G模块接收到的消息
 * 参数：	空
 * 返回：	空
 */
u8 MSG_DealUartMsg(void);

void MSG_SendCanMsgToBt(void);

#endif
