#ifndef __UDS_UPGRADE_H__
#define __UDS_UPGRADE_H__

#include "common.h"

typedef enum {
	UDS_UPGRADE_START 	= 1,
	UDS_UPGRADE_CONTENT = 2,
	UDS_UPGRADE_END	= 3,
	UDS_UPGRADE_ERASE 	= 4
} UDS_UPGRADE_TYPE;

#pragma pack (1) // 指定按一字节对齐

typedef struct {
	u32 startAddr; 	// 起始地址
	u32 totalSize;	// 文件大小
} uds_upgrade_start_t;

typedef struct {
	u32 addr;
	u8 size;
	u8 data;
} uds_upgrade_content_t;

typedef struct {
	u32 runAddr; // 运行地址
 	u32 fileCrc; // 文件crc
} uds_upgrade_end_t;

#pragma pack () // 取消指定结构体对齐

void UDS_Upgrade(u8 *data, u8 size);
















#endif
