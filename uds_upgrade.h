#ifndef __UDS_UPGRADE_H__
#define __UDS_UPGRADE_H__

#include "common.h"

typedef enum {
	UDS_UPGRADE_START 	= 1,
	UDS_UPGRADE_CONTENT = 2,
	UDS_UPGRADE_END	= 3,
	UDS_UPGRADE_ERASE 	= 4
} UDS_UPGRADE_TYPE;

#pragma pack (1) // ָ����һ�ֽڶ���

typedef struct {
	u32 startAddr; 	// ��ʼ��ַ
	u32 totalSize;	// �ļ���С
} uds_upgrade_start_t;

typedef struct {
	u32 addr;
	u8 size;
	u8 data;
} uds_upgrade_content_t;

typedef struct {
	u32 runAddr; // ���е�ַ
 	u32 fileCrc; // �ļ�crc
} uds_upgrade_end_t;

#pragma pack () // ȡ��ָ���ṹ�����

void UDS_Upgrade(u8 *data, u8 size);
















#endif
