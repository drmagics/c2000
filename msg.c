#include "msg.h"
#include "uart.h"
#include "update.h"
#include "power.h"
#include "can_manager.h"
#include "delay.h"
#include "time_task.h"
#include "bt_manager.h"
#include "uds_manager.h"
#include "ccp_manager.h"
#include "uds_upgrade.h"

extern void  MSG_DealUds(TYPE_Can *canMsg) ;

// 4G模块信息
TYPE_AppInfo g_MsgAppInfo;

// 需要Can发送的状态信息
TYPE_AppStatus g_MsgAppStatus;
u32 g_MsgAppStatusRecvTime = 0;	// 接收状态信息时间

// Can转接开关
u8 g_MsgCanBridgeSwitch = 0;
u32 g_MsgCanBridgeSwitchTime = 0;

// 4G模块是否空闲
boolean g_Msg4gIsFree = FALSE;

void MSG_Init()
{
	memset(&g_MsgAppInfo, 0, sizeof(g_MsgAppInfo));
	memset(&g_MsgAppStatus, 0, sizeof(g_MsgAppStatus));
}

#define DEBUG_TYPE_LOG_INFO		0
#define DEBUG_TYPE_GET_VIN		1
#define DEBUG_TYPE_SET_VIN		2
#define DEBUG_TYPE_GET_VER		3

#define DEBUG_ACK_OK			1
#define DEBUG_ACK_ERR			0


void MSG_DealUartCanMsg(u8 *buff, u8 len)
{
	TYPE_Can *canMsg = (TYPE_Can *)buff;
	canMsg->id	= BigLittleSwap32(canMsg->id);
	
	CAN_SendExtMsg(canMsg);	
}


/**
 *	处理can报文数据
 */
#define MSG_LOST_CAN_MSG_TIME		10000
boolean g_MsgIsLostCanMsg = FALSE;
#include "led.h"
void MSG_CheckCanMsg()
{
	static u32 lostCanMsgLastTime	= 0;
	static boolean lostCanMsgFlag = FALSE;
	if(CAN_RingIsEmpty()) {
		if(!lostCanMsgFlag) {
			lostCanMsgLastTime = g_SysTicks;
			lostCanMsgFlag = TRUE;
		} else if(g_SysTicks - lostCanMsgLastTime > MSG_LOST_CAN_MSG_TIME) {
			g_MsgIsLostCanMsg = TRUE;
		}
	} else {
		lostCanMsgFlag = FALSE;
		g_MsgIsLostCanMsg = FALSE;
	}
	
	
	if(!g_MsgIsLostCanMsg) {
		LED1_SlowFlash();
	} else {
		LED1_QuickFlash();
	}
	
}


#define MAX_SEND_CAN_MSG_LEN	5

#define CAN_PKG_SEND_INTERVAL		100


void MSG_DealCanMsg()
{
	TYPE_Can canMsg;
	u8 i = 0;
	bt_can_pkg_t *pBtCanPkg;
	MSG_CheckCanMsg();
	while(CAN_GetRingItem(&canMsg)) {
		if((canMsg.id == CAN_MSG_ID_UDS) || (canMsg.id == CAN_VCU_RESET_ID) ) {
			MSG_DealUds(&canMsg);
			continue;
		}
		
		if(ccp_deal_can_msg(&canMsg) != 0) {
			continue;
		} 
	#if 0
		LOGDln("canId:0x%08X,ch:%d", canMsg.id, canMsg.ch);
	#endif	

		if(BT_IS_CONNECT()) {
			for(i = 0; i < g_BtCanPkgCount; i++) {
				pBtCanPkg = &g_BtCanPkgs[i];
				if(pBtCanPkg->canId == canMsg.id) {
					memcpy(&pBtCanPkg->canPkg, &canMsg, sizeof(TYPE_Can));
					pBtCanPkg->isUpdate = TRUE;
					break;
				}
			}
			
			if(i == g_BtCanPkgCount) {
				pBtCanPkg = &g_BtCanPkgs[i];
				pBtCanPkg->canId = canMsg.id;
				memcpy(&pBtCanPkg->canPkg, &canMsg, sizeof(TYPE_Can));
				pBtCanPkg->isUpdate = TRUE;
				g_BtCanPkgCount++;
			}
			
		}
	}
	MSG_SendCanMsgToBt();
}

//u8 g_BtCanSendFlag = 1;

void MSG_SendCanMsgToBt()
{
	TYPE_Can canMsgs[MAX_SEND_CAN_MSG_LEN];
	u8 i = 0, j = 0;
	bt_can_pkg_t *pBtCanPkg;
	static u8 startIndex = 0;
	
#if 0	
	if(!g_BtCanSendFlag) {
		return;
	}
#endif	
	
	startIndex = startIndex >= g_BtCanPkgCount ? 0 : startIndex;
	
	for(i = 0, j = 0; i < g_BtCanPkgCount; i++) {
		pBtCanPkg = &g_BtCanPkgs[(i + startIndex) % MAX_CAN_PKG_COUNT];
		
		if(pBtCanPkg->isUpdate 
			&& g_SysTicks > pBtCanPkg->lastSendTimestamp 
			&& g_SysTicks - pBtCanPkg->lastSendTimestamp >= CAN_PKG_SEND_INTERVAL) {
			pBtCanPkg->isUpdate = FALSE;
			pBtCanPkg->lastSendTimestamp = g_SysTicks;
//			LOGDln("%d,0x%08X", g_SysTicks, pBtCanPkg->canId);
				
			memcpy(&canMsgs[j], &pBtCanPkg->canPkg, sizeof(TYPE_Can));
			j++;
			if(j >= MAX_SEND_CAN_MSG_LEN) {
				BT_SendCanMsg(canMsgs, j);
				j = 0;			
				startIndex = (i + startIndex + 1) % MAX_CAN_PKG_COUNT;
				break;
			}
		}
	}
	
	if(j > 0) {
		BT_SendCanMsg(canMsgs, j);
		startIndex = (i + startIndex + 1) % MAX_CAN_PKG_COUNT;
	}
}


void MSG_DealCCPConfig(u8 *buff, u8 len)
{
	ccp_config(buff, len);
}

void MSG_DealCCPMsg(u8 *buff, u8 len)
{
	ccp_start(buff, len);
}

void MSG_DealUdsMsg(u8 *buff, u8 len)
{
//	 order = buff[0];
//	 LOGDln("order:%d", order);
/*	
	switch(order)
	{
		case 1:modify_session_flag = 1 ;break;
		case 2:secure_session_flag = 1 ;break;
	  case 3:verify_session_flag = 1 ;break;
	  case 4:Write_Vin_flag = 1 ;	break;
	}
*/   
}

void MSG_ReadVin()
{
//	order = 5;
//	{
//	 	Read_Vin_flag = 1 ;			    	
//	}
	//if(read_success_flag ==1)
	//{
	//	LOGDln("read vin :%s",READ_VIN);
	//  read_success_flag = 0;
	//}
}
void MSG_WriteVin(u8 *buff, u8 len)
{
	static char vin[18];
	memcpy(vin, buff, 17);
	vin[17] = '\0';
	LOGDln("write vin:%s", vin);
	
	UDS_WriteVin(vin);
  //LOGDln("order = 1 \r\n");
}

#define UPDATE_FLASH_FLAG		0x9096

void MSG_DoUpgradeReboot() {
	LOGDln("upgrade reboot");
	u16 upgradeFlag = UPDATE_FLASH_FLAG;
	STMFLASH_Write(UPDATE_FLASH_CONFIG_ADDR, &upgradeFlag, 1);
	__set_FAULTMASK(1);      // 关闭所有中端
	NVIC_SystemReset();// 复位
}

#define UART_RECV_CACHE_SIZE 	256
u8		recv_cache_buf[UART_RECV_CACHE_SIZE];
u16     recv_cache_len = 0;
/**
 * 处理从4G模块接收到的消息
 */
u8 MSG_DealUartMsg(void)
{
    u16             	data_len;
    TYPE_CommonHeader 	*header;
    u16             	i, offset;
    u8           		calcCrcCode;
	u16					dataIndex;
	u16					headerLen = sizeof(TYPE_CommonHeader);
	u16 				crcCodeIndex = 0;
	// 30s内没有接收到4G数据
//	if(g_SysTicks - g_MsgCanBridgeSwitchTime > MSG_CAN_BRIDGE_TIMEOUT) {
//		g_MsgCanBridgeSwitch = 0;	// 关闭CAN桥接开关
//	}
	
    data_len = BT_RecvMsg(&(recv_cache_buf[recv_cache_len]));
	recv_cache_len += data_len;
    if (recv_cache_len <= 0) {
        return 0;
    }
  
//	printArray(recv_cache_buf, recv_cache_len);
    offset = 0;
	while (offset + headerLen <= recv_cache_len) {
        header = (TYPE_CommonHeader *)(&recv_cache_buf[offset]);
		if (header->startTag == COMMON_HEADER_START_TAG) {
            
            if (offset + headerLen + header->len + 1 > recv_cache_len) {
                break;
            }
            crcCodeIndex = offset + headerLen + header->len;
			calcCrcCode = calcCrc((u8 *)header, 2, headerLen - 2 + header->len);
            if (recv_cache_buf[crcCodeIndex] != calcCrcCode) {
                LOGEln("CRC check err:0x%02X,0x%02X", recv_cache_buf[crcCodeIndex], calcCrcCode);
//				LOGEln("Offset:%d,len:%d,index:%d", offset, header->len, crcCodeIndex);
                for (i = offset; i < offset + headerLen + header->len + 1; i++) {
                    LOGD("%02x ", recv_cache_buf[i]);
                }
                LOGDln();
                recv_cache_len = 0;
                break;
            }
			
			dataIndex = offset + sizeof(TYPE_CommonHeader);
            switch (header->cmd) {
			
			case BT_TYPE_CCP:
				MSG_DealCCPMsg(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case BT_TYPE_CCP_CONFIG:
				MSG_DealCCPConfig(&recv_cache_buf[dataIndex], header->len);
				break;
				
			case BT_TYPE_UDS:
				MSG_DealUdsMsg(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case BT_TYPE_VIN_READ:
				MSG_ReadVin();
				break;
			
			case BT_TYPE_VIN_WRITE:
				MSG_WriteVin(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case BT_TYPE_TROUBLE:
				UDS_Trouble(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case BT_TYPE_UPDATE:
				UPDATE_ByUart(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case BT_TYPE_UDS_UPGRADE:
				UDS_Upgrade(&recv_cache_buf[dataIndex], header->len);
				break;
			
			case 0x11:
				LOGDln("%d", recv_cache_buf[dataIndex]);
			break;
			
            default:
				LOGDln("recv unknow message ACK %d", header->cmd);
                break;
            }

            offset += sizeof(TYPE_CommonHeader) + header->len + 1;

		} else {
			offset++;
		}
    }
	
	if (offset > 0) {
		memmove(recv_cache_buf, &recv_cache_buf[offset], recv_cache_len - offset);
	}
	recv_cache_len -= offset;

    if (recv_cache_len > UART_RECV_CACHE_SIZE) {
        recv_cache_len = 0;
    }

    return 1;	
}

