#ifndef __CHENJUN_MSG_H__
#define __CHENJUN_MSG_H__

#include "common.h"
#include "can.h"
#include "rtc.h"
#include "timer.h"
#include "time_task.h"
#include "ccp_manager.h"


// ������Ϣ��4G
#define MSG_TO_4G_CAN					0x01	// CAN��Ϣ����
#define MSG_TO_4G_UPDATE_MCU			0x20
#define MSG_TO_4G_UPDATE_4G				0x21	

// ��4G������Ϣ
#define MSG_FROM_4G_CAN					0x90	// CAN��Ϣ����
#define MSG_FROM_4G_UPDATE_MCU			0xA0
#define MSG_FROM_4G_UPDATE_4G			0xA1

#define MSG_CAN_BRIDGE_TIMEOUT			10000	// 10000 ms = 10s δ���յ�4G��������ֹͣ����can����


void MSG_Init(void);

/**
 * ���ܣ�	����can��������
 * ������	��
 * ���أ�	��
 */
void MSG_DealCanMsg(void);

/**
 * ���ܣ�	�����4Gģ����յ�����Ϣ
 * ������	��
 * ���أ�	��
 */
u8 MSG_DealUartMsg(void);

void MSG_SendCanMsgToBt(void);

#endif
