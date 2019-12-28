#ifndef __CHENJUN_UPDATE_H__
#define __CHENJUN_UPDATE_H__


#include "common.h"
#include "stmflash.h"

#define UPDATE_FLASH_APP_ADDR			0x8003000
#define UPDATE_FLASH_PKG_ADDR			0x800B000
#define UPDATE_FLASH_CONFIG_ADDR		0x8013000

#define UPDATE_UART_MAX_SIZE		240

#define UPDATE_TYPE_START		0x00
#define UDPATE_TYPE_CTL			0x01
#define UPDATE_TYPE_DATA		0x02
#define UPDATE_TYPE_DONE		0x03

#define UPDATE_ACK_SUCCESS		0x01
#define UPDATE_ACK_FAILED		0x00

typedef enum {
	DEV_SBOX = 0x01,
	DEV_VCU = 0x02,
} update_dev_t; 

#define EN_CAN_UPDATE		0
#define EN_UART_UPDATE		1

#pragma pack(1)  // 让编译器做1字节对齐

typedef struct {
	u16 updateFlag;
	u16 fileSize;
	u16 appRunFlag;
} UpdateInfo_t;

#define UPDATE_FLASH_FLAG		0x9096
#define APP_RUN_FLAG			0x6699

typedef struct {
	u8 type;
	u8 dev;
	u8 ack;
	union content {
		u16 totalSize;
		u16 offset;
		u16 fileCrc;
	} content;
} UPDATE_UartHeader_t;


#pragma pack() // 取消1字节对齐，恢复为默认对齐

void UPDATE_WriteAppRunFlag(void);

void UPDATE_Init(void);
void UPDATE_ByUart(u8 *buf, u8 len);

#endif

