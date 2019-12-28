#include "update.h"
#include "msg.h"
#include "can.h"
#include "type.h"
#include "delay.h"
#include "bt_manager.h"
#include "can_manager.h"

u8  g_updateMode = 0;			// 升级模式	（1  串口升级， 2  CAN升级）
u16 g_updateFileSize;		// 升级文件大小
u16 g_updateFileOffset;		// 升级文件偏移值
u16 g_updateFileCrc;		// 升级文件CRC

u8   g_updatePkgCrc; 		// 每组CAN数据包CRC
u16  g_updatePkgSize;		// 每组数据包大小	

u8  g_updateBuff[STM_SECTOR_SIZE];	
u32 g_updateBuffOffset;	
u16 g_updateWriteCount;


void UPDATE_SuccessDoReset(u16 fileSize);

void writeFlash(u8 *buf, u8 len)
{
	if(g_updateMode == 0) {
		return;
	}
#ifdef BOOTLOADER
	u32 writeAddr = UPDATE_FLASH_APP_ADDR + g_updateWriteCount * STM_SECTOR_SIZE;
#else 
	u32 writeAddr = UPDATE_FLASH_PKG_ADDR + g_updateWriteCount * STM_SECTOR_SIZE;
#endif
	LOGDln("123g_updateBuffOffset:%08X,len:%08X",g_updateBuffOffset,len);
	if(g_updateBuffOffset + len >= STM_SECTOR_SIZE) {
		u16 cpyLen = STM_SECTOR_SIZE - g_updateBuffOffset;
		
		memcpy(&g_updateBuff[g_updateBuffOffset], &buf[0], cpyLen);
	
		LOGDln("Write flash, addr:%08X,g_updateBuffOffset:%08X", writeAddr,g_updateBuffOffset);
		STMFLASH_Write(writeAddr, (u16 *)g_updateBuff, STM_SECTOR_SIZE / sizeof(u16));
	
		g_updateWriteCount++;		
		LOGDln("write over");
		if(len > cpyLen) {
			memcpy(&g_updateBuff[0],  &buf[cpyLen], len - cpyLen);
		}
		g_updateBuffOffset = len - cpyLen;
		g_updateFileOffset += len;
	} else {
		memcpy(&g_updateBuff[g_updateBuffOffset], &buf[0], len);
		g_updateBuffOffset += len;
		g_updateFileOffset += len;
		LOGDln("456g_updateBuffOffset:%08X,len:%08X",g_updateBuffOffset,len);
		if(g_updateFileOffset == g_updateFileSize) {
			// 是最后一帧数据包
			LOGDln("Writeee flash, addr:%08X", writeAddr);
			STMFLASH_Write(writeAddr, (u16 *)g_updateBuff, g_updateBuffOffset / sizeof(u16));		
			g_updateWriteCount++;
			LOGDln("writeee over");
		}
	}
}


void UPDATE_UartSendAck(u8 ack, UPDATE_UartHeader_t *header)
{
	header->ack = ack;
	BT_SendMsg(BT_ACK_UPDATE, header, sizeof(UPDATE_UartHeader_t));
}


void UPDATE_ByUart(u8 *buf, u8 len)
{
	u16 i;
	UPDATE_UartHeader_t *header = (UPDATE_UartHeader_t *)buf;
	if(header->dev != DEV_SBOX) {
		return;
	}
	
	switch(header->type) {
		case UPDATE_TYPE_START:
			g_updateFileSize = BigLittleSwap16(header->content.totalSize);
			g_updateFileOffset = 0;
			g_updateFileCrc = 0;
			g_updateBuffOffset = 0;
			g_updateWriteCount = 0;
			g_updateMode = 1;
			LOGDln("Download start, file size:%d", g_updateFileSize);
			UPDATE_UartSendAck(UPDATE_ACK_SUCCESS, header);
			CAN_Disable();
			break;
		
		case UPDATE_TYPE_DATA:
			for(i = sizeof(UPDATE_UartHeader_t); i < len; i++) {
				g_updateFileCrc ^= buf[i];
			}
			LOGDln("g_updateFileOffset:%08X,header->content.offset:%08X", g_updateFileOffset,BigLittleSwap16(header->content.offset));
			if(g_updateFileOffset == BigLittleSwap16(header->content.offset)) {
			
				LOGDln("abcg_updateBuffOffset:%08X ,size:%08X", g_updateBuffOffset,len - sizeof(UPDATE_UartHeader_t));
				writeFlash(&buf[sizeof(UPDATE_UartHeader_t)], len - sizeof(UPDATE_UartHeader_t));	
				UPDATE_UartSendAck(UPDATE_ACK_SUCCESS, header);
			} else {
				LOGDln("Offset error:%d, %d", g_updateFileOffset, BigLittleSwap16(header->content.offset));
				UPDATE_UartSendAck(UPDATE_ACK_FAILED, header);
				CAN_Enable();
			}
			break;
		
		case UPDATE_TYPE_DONE:
			if(g_updateFileCrc == BigLittleSwap16(header->content.fileCrc)
				&& g_updateFileSize == g_updateFileOffset) {
				// 下载文件成功
				UPDATE_UartSendAck(UPDATE_ACK_SUCCESS, header);	
					LOGDln("Upgrade suceess");
				UPDATE_SuccessDoReset(g_updateFileSize);
			} else {
				LOGEln("Download failed\r\n" 
						"crc :0x%02x, 0x%02x\r\n"
						"size :%d, %d", 
					g_updateFileCrc, BigLittleSwap16(header->content.fileCrc),
					g_updateFileSize, g_updateFileOffset);
				UPDATE_UartSendAck(UPDATE_ACK_FAILED, header);
			}
			CAN_Enable();
			break;
	}
}


void UPDATE_WriteAppRunFlag()
{
	UpdateInfo_t updateInfo;
	
	STMFLASH_Read(UPDATE_FLASH_CONFIG_ADDR, (u16 *)&updateInfo, sizeof(updateInfo) / 2);
		
	if(updateInfo.appRunFlag != APP_RUN_FLAG) {
		updateInfo.updateFlag = 0xFFFF;
		updateInfo.appRunFlag = APP_RUN_FLAG;
		STMFLASH_Write(UPDATE_FLASH_CONFIG_ADDR, (u16 *)&updateInfo, sizeof(updateInfo) / 2);
		LOGDln("updateInfo.appRunFlag:%x", updateInfo.appRunFlag);
	}
}

void UPDATE_SuccessDoReset(u16 fileSize)
{	
	UpdateInfo_t updateInfo;
	
	updateInfo.updateFlag = UPDATE_FLASH_FLAG;
	updateInfo.fileSize = fileSize;
	updateInfo.appRunFlag = 0xFFFF;
	// 更新bootloader 标识信息
	STMFLASH_Write(UPDATE_FLASH_CONFIG_ADDR, (u16 *)&updateInfo, sizeof(updateInfo) / 2);	
  //STMFLASH_Read(UPDATE_FLASH_CONFIG_ADDR, (u16 *)&updateInfo, sizeof(updateInfo) / 2);	
  //LOGDln("updateInfo.appRunFlag:%x ,updateInfo.updateFlag :%x", updateInfo.appRunFlag,updateInfo.updateFlag);
	LOGDln("download success");
	delay_ms(100);

	__set_FAULTMASK(1);      // 关闭所有中端
	NVIC_SystemReset();// 复位
}

