#include "uds_upgrade.h"
#include "bt_manager.h"
#include "uds_manager.h"
#include "ISOTP.h"
#include "UDS.h"
void UDS_dealStart(uds_upgrade_start_t *pHeader);
void UDS_dealContent(uds_upgrade_content_t *pContent, u8 size);
void UDS_dealEnd(uds_upgrade_end_t *pEnd);
void UDS_upgrade_ack(u8 cmd, u8 ack);
void UDS_upgrade_recv_callback(uds_recv_t *pRecv);




/**
 * 处理uds升级数据
 */
void UDS_Upgrade(u8 *data, u8 size)
{
	u8 cmd = data[0];
	switch(cmd) {
		case UDS_UPGRADE_START:
			uds_connect_init();
		    uds_task_flag |=  BLE_UDS_CMD_UP_VCU ;
		    uds_upgrade_success_flag = 0 ;
			UDS_dealStart((uds_upgrade_start_t *)&data[1]);
			break;
		
		case UDS_UPGRADE_CONTENT:
			UDS_dealContent((uds_upgrade_content_t *)&data[1], size - 1);
			break;
		
		case UDS_UPGRADE_END:
			UDS_dealEnd((uds_upgrade_end_t *)&data[1]);
			break;
		
		default:
			break;
	}
}
void delay_ms(u16 nms);
void UDS_dealStart(uds_upgrade_start_t *pStart)
{
	pStart->startAddr = BigLittleSwap32(pStart->startAddr);
	pStart->totalSize = BigLittleSwap32(pStart->totalSize);
	
	uint8_t  i ,j; 
    for(i=0;i<4;i++)
    {
      UpgProgAddr[i]=(pStart->startAddr>>((3-i)*8))& 0xFF;
    }
	
	for(j=0;j<4;j++)
    {
      UpgProgSize[j]=(pStart->totalSize>>((3-j)*8))& 0xFF;
    }
	
	
	
	LOGDln("UpgProgAddr：%x,%x,%x,%x UpgProgSize:%x,%x,%x,%x \r\n", UpgProgAddr[0],UpgProgAddr[1],UpgProgAddr[2],UpgProgAddr[3],UpgProgSize[0],UpgProgSize[1],UpgProgSize[2],UpgProgSize[3]);
	LOGDln("uds upgrade start, addr:0x%08X, size:%d", pStart->startAddr, pStart->totalSize);
	
	uds_status = UDS_UPG_ERASE_FLASH ;
	
//  UDS_setRecvCallback(UDS_upgrade_recv_callback);
	
//	UDS_upgrade_ack(UDS_UPGRADE_START, 1);
//	delay_ms(2000);
//	UDS_upgrade_ack(UDS_UPGRADE_ERASE, 1);
	
	
}

void UDS_dealContent(uds_upgrade_content_t *pContent, u8 size)
{
	pContent->addr = BigLittleSwap32(pContent->addr);
	sendsize =  pContent->size ;
	//pContent->data = BigLittleSwap32(pContent->data);
	
	
	if(pContent->size + 5 != size) {
		LOGEln("uds upgrade data error:%d,%d", pContent->size, size);
		return;
	}
	
	
	uint8_t  i ,j; 
    for(i=0;i<4;i++)
    {
      UpgProgAddr[i]=(pContent->addr>>((3-i)*8))& 0xFF;
    }
	
	for(j=0;j<4;j++)
    {
      UpgProgSize[j]=(pContent->size>>((3-j)*8))& 0xFF;
    }
	
	//memset(&Download_Data_Sendout[2], 0, 318);
	memcpy(&Download_Data_Sendout[2], &pContent->data, pContent->size);
	
	LOGDln("uds upgrade run, addr:0x%08X, size:%d, %02x,%02x", pContent->addr, pContent->size,
			Download_Data_Sendout[2], Download_Data_Sendout[3]);
	uds_upgrade_state = uds_upgrade_start ;
//	UDS_upgrade_ack(UDS_UPGRADE_CONTENT, 1);
}

void UDS_dealEnd(uds_upgrade_end_t *pEnd)
{
	pEnd->runAddr = BigLittleSwap32(pEnd->runAddr);
	pEnd->fileCrc = BigLittleSwap32(pEnd->fileCrc);
	
	uint8_t  i ; 
    for(i=0;i<4;i++)
    {
      UpgCrcCode[i]=(pEnd->fileCrc>>((3-i)*8))& 0xFF;
    }
	
	LOGDln("uds upgrade end, addr:0x%08X, crc:0x%08X", pEnd->runAddr, pEnd->fileCrc);
	
	uds_status = UDS_UPG_DOWN_END ;
	
	
	
//	UDS_upgrade_ack(UDS_UPGRADE_END, 1);
}


void UDS_upgrade_ack(u8 cmd, u8 ack)
{
	u8 arr[2];
	
	arr[0] = cmd;
	arr[1] = ack;
	BT_SendMsg(BT_ACK_UDS_UPGRADE, arr, sizeof(arr));
}

void UDS_upgrade_recv_callback(uds_recv_t *pRecv)
{
	if(pRecv->server != 0x31) {
		return;
	}
}


