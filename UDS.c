#include <stdio.h>
#include <string.h>
#include "UDS.h"
#include "stm32f10x_flash.h"
#include "bt_manager.h"
#include "uds_manager.h"
#include "delay.h"
#include "uds_upgrade.h"

#define MASK			0x45574159                      /* 安全访问掩码 */
#define FlashStartAddr_VIN	0x08008000                      /* 定义Flash初始化数据地址 */
#define Flash_Save_Date_LEN	VIN_LEN + MIN_LEN + CQ_LEN + MKDATE_LEN
uint8_t VIN_Receive_Comp_Flag		= 0;
uint8_t VIN_Receive_Respond_Flag	= 0;                    /* 响应标志 */
uint8_t VIN_Save_Star_Flag		= 0;                    /* vin储存开始 */
uint8_t MIN_Save_Star_Flag		= 0;                    /* MIN开始标志 */
uint8_t UDS_Receive_Status		= 0;                    /* UDS建立会话 */
uint8_t UDS_Verify_Status		= 0;                    /* UDS建立会话 */
uint8_t UDS_SafeRequest_Status		= 0;                    /* 安全访问状态 */
uint8_t SEND_VIN[17];
uint8_t READ_VIN[17];
uint8_t uds_task_flag =0;
uint8_t uds_connect_status = UDS_STATUS_DISCONNECT;		//  uds连接状态
uint8_t uds_status    =  0 ;
uint8_t Send_Sl_DTC_flag   = 0;
uint8_t send_success_flag  = 0;
uint8_t Clear_Sl_DTC_flag  = 0;
uint8_t clear_success_flag = 0;
uint8_t Read_DTC_flag      = 0;
uint8_t read_DTC_success_flag= 0;
uint8_t Read_DTC_Cnt= 0;
uint8_t Clear_DTC_flag= 0;
uint8_t clear_DTC_success_flag = 0;
uint8_t request_earseflash_flag  = 0;
uint8_t requset_download_flag  = 0;
uint8_t sendsize  = 0;
uint8_t UDS_DTC_CODE[8] ={0};
uint32_t UDS_PHY_AD_ID = 0x18dad0f9 ;
uint32_t UDS_FUN_AD_ID = 0x18dbd0f9 ;
uds_task_t uds_task;

uint8_t  UpgProgAddr[4] = {0} ;
uint8_t  UpgProgSize[4] = {0} ;
uint8_t  UpgCrcCode[4] =  {0} ;
uint8_t  uds_upgrade_state = 0 ;
uint8_t  mFrameBuf[320] = {0} ;
uint8_t  send_vcu_reset_flag = 0 ;
uint8_t  uds_upgrade_success_flag = 0 ;

uint8_t		Read_Flash_VIN_MIN[Flash_Save_Date_LEN];        /* 接受到的VIN和电机编号 */
uint8_t		EEPROM_DTC_Cnt	= 0;                            /* 历史故障的个数 */
uint32_t	UDS_RESPOND_ID	= 0;

extern u8 g_BtCanSendFlag;
extern uint8_t	order;
UDS_Time	Time;                                           /* UDS储存的时间戳 */
/* flash vin */

FlashData	Flash;
uint8_t		VIN_READ_Flag	= TRUEE;                        /* 开机的时候读取内部存储的VIN */
uint8_t		VIN_SAVE_Flag	= 0;
/*  */
uint8_t Time_refresh = 0;


uint8_t ErreaseVIN_Response[8] =
{
	0x50, 0x03, 0x00, 0x32, 0x01, 0xf4
};
uint8_t UDS_Safe_Response4[8] =         /* 安全访问回复 申请验证码等待回复 */
{
	0x67, 0x01, 0xe6, 0x8e, 0x09, 0x8b
};
uint8_t UDS_Negti_Response4[8] =        /* 安全访问回复 */
{
	0x67, 0x01, 0xe6, 0x8e, 0x09, 0x8b
};
uint8_t ErreaseVIN_Response2[8] =
{
	0x6E, 0xF1, 0x90, 0
};
uint8_t ErreaseVIN_Response3[31] =
{
	0x62, 0xF1, 0x90,
};
uint8_t ErreaseVIN_Response5[8] =
{
	0x67, 0x02,
};
uint8_t Modify_Session_Sendout[8] =
{
	0x10, 0x03,
};
uint8_t Modify_Session_Sendout1[8] =
{
	0x10, 0x02,
};

uint8_t Modify_Session_Sendout2[8] =
{
	0x10, 0x01,
};
uint8_t ErreaseVIN_Sendout1[8] =
{
	0x27, 0x01,
};
uint8_t ErreaseVIN_Sendout2[8] =
{
	0x27, 0x02, 0xa3, 0xd9, 0x48, 0xd2,
};
uint8_t ErreaseVIN_Sendout3[21] =
{
	0x2e, 0xf1, 0x90,
};
uint8_t ErreaseVIN_Sendout4[8] =
{
	0x22, 0xf1, 0x90,
};
uint8_t ErreaseVIN_Sendout5[8] =
{
	0
};

uint8_t SL_DTC_Sendout[8] =
{
	0x2e ,0xf2 ,0x91,0
};

uint8_t SL_DTC_Sendout1[8] =
{
	0x14 ,0xfa ,
};
uint8_t READ_DTC[8] =
{
	0x19 ,0x01 ,
};
uint8_t DTC_Sendout1[8] =
{
	0x14 ,0xff ,
};

uint8_t COMM_Sendout[8] =
{
	0x28 ,0x03 ,0x03 ,
};

uint8_t COMM_Sendout1[8] =
{
	0x28 ,0x00 ,0x03 ,
};

uint8_t Request_Erase_Sendout[16] =
{
	0x31 ,0x01 ,0x03 ,0x03 ,0x44 , 
};

uint8_t Request_download_Sendout[16] =
{
	0x34 ,0x01 ,0x44 , 
};
uint8_t Download_Data_Sendout[320] =
{
	0x36 ,0x01 , 
};
uint8_t Download_End_Sendout[8] =
{
	0x37 ,0x01 , 
};
uint8_t UDS_CRC_Sendout[8] =
{
	0x31 ,0x01 , 0x02 ,0x02 ,
};
uint8_t VCU_RESET_Sendout[8] =
{
	0x11 ,0x01 ,
};
uint8_t UDS_DTC_Resppond[8] = { 0 };



void Nagetive_Respond( uint8_t *buff_code )
{
	G_ISO_TP.Request( UDS_RESPOND_ID, ErreaseVIN_Response, 6 );                                     /* 回复响应 */
}


uint8_t read_vin_checkpin	= 0;




void UDS_SREVE(void)
{
	
	if(uds_task_flag & BLE_UDS_CMD_CONNECT
	    && uds_connect()){
		uds_task_flag &= ~BLE_UDS_CMD_CONNECT;
	}
	  
	if(uds_task_flag & BLE_UDS_CMD_WRITE_VIN     
		&& uds_write_vin()) {                       //写VIN
		uds_task_flag &= ~BLE_UDS_CMD_WRITE_VIN;
	}
	
//	if(uds_task_flag & BLE_UDS_CMD_READ_VIN      
//		&& ccp_request_monitor()) { 		        //读VIN
//		uds_task_flag &= ~BLE_UDS_CMD_READ_VIN; 	
//	} 
	
	if(uds_task_flag & BLE_UDS_CMD_SEND_SL_DTC 
		&& uds_send_sl_dtc()) {			            // 发送模拟故障	
		uds_task_flag &= ~BLE_UDS_CMD_SEND_SL_DTC;
	}
	
	if(uds_task_flag & BLE_UDS_CMD_DEL_SL_DTC
		&& uds_clear_SL_DTC()) {			        // 清除模拟故障
		uds_task_flag &= ~BLE_UDS_CMD_DEL_SL_DTC;
	}
	
	if(uds_task_flag & BLE_UDS_CMD_READ_DTC
		&& uds_read_DTC()) {		                // 读取故障码
		uds_task_flag &= ~BLE_UDS_CMD_READ_DTC;
	}

	if(uds_task_flag & BLE_UDS_CMD_DEL_DTC
		&& uds_clear_DTC()) {		                // 清除故障码
		uds_task_flag &= ~BLE_UDS_CMD_DEL_DTC;
	}
    
	if(uds_task_flag & BLE_UDS_CMD_UP_VCU
		&& uds_up_vcu()) {		                     // 升级VCU
		uds_task_flag &= ~BLE_UDS_CMD_UP_VCU;
	}




}	

uint8_t uds_is_connected() {
	return uds_connect_status == UDS_STATUS_CONNECTED;
}



uint8_t uds_connect()
{
	if(uds_connect_status != UDS_STATUS_DISCONNECT) {
		return 0;
	 }
	else {	
	  
		if (order == 1 ) {
		  if ( modify_session_flag == 0 ) {
			   G_ISO_TP.Request( UDS_PHY_AD_ID, Modify_Session_Sendout, 2 ); /* 发送命令 */
			   modify_session_flag = 1;
		  }

		  if ( buff_read_flag == 1 ) {
			   buff_read_flag = 0;
			 if ( buff[0] == 0x50 && buff[1] == 0x03 && buff[2] == 0x00 && buff[3] == 0x32 && buff[4] == 0x01 && buff[5] == 0xF4 ) {
				if(uds_task_flag & BLE_UDS_CMD_UP_VCU) {
				   order  = 4;	
		        }
		        else {
		           order  = 2;
//				   printf( "order = 2 \r\n" );
		        }
			 }
		  }
	    }
		
		if (order == 4 ) {
		  if ( modify_funsession_flag == 0 ) {
			   G_ISO_TP.Request( UDS_FUN_AD_ID, Modify_Session_Sendout, 2 ); /* 发送命令 */
			   modify_funsession_flag = 1;
		  }

		  if ( buff_read_flag == 1 ) {
			   buff_read_flag = 0;
			 if ( buff[0] == 0x50 && buff[1] == 0x03 && buff[2] == 0x00 && buff[3] == 0x32 && buff[4] == 0x01 && buff[5] == 0xF4 ) {
				  order  = 5;
			 }
		  }
		}
	    
		if (order == 5) {
		  if ( comm_funsession_flag == 0 ) {
			   G_ISO_TP.Request( UDS_FUN_AD_ID, COMM_Sendout, 3 ); /* 发送命令 */
			   comm_funsession_flag = 1;
		  }

		  if ( buff_read_flag == 1 ) {
			   buff_read_flag = 0;
			 if ( buff[0] == 0x68 && buff[1] == 0x03 ) {
				  order  = 6;
			 }
		  }
		}
		
		if (order == 6) {
		  if ( modify_session_flag1 == 0 ) {
			   G_ISO_TP.Request( UDS_PHY_AD_ID, Modify_Session_Sendout1, 2 ); /* 发送命令 */
			   modify_session_flag1 = 1;
		  }

//		  if ( buff_read_flag == 1 ) {
//			   buff_read_flag = 0;
//			 if ( buff[0] == 0x50 && buff[1] == 0x02 && buff[2] == 0x00 && buff[3] == 0x32 && buff[4] == 0x01 && buff[5] == 0xF4 ) {
//				  order  = 7;
//			      printf("order  = 7 \r\n");
//			 }
//		  }
	     }
		if ( start_secure_flag == 1) {
		     order  = 2;
		     start_secure_flag = 0;
		}
		
		
		
		if ( order == 2 ) {
		 
			if ( secure_session_flag == 0 )
			{
				printf("order  = 2 \r\n");
				G_ISO_TP.Request( UDS_PHY_AD_ID, ErreaseVIN_Sendout1, 2 );     /* 发送命令 */
				secure_session_flag = 1;
			} 
		  
		  if ( buff_read_flag == 1 ) {
			   buff_read_flag = 0;
			if ( buff[0] == 0x67 && buff[1] == 0x01 && buff[2] == 0xE6 && buff[3] == 0x8E && buff[4] == 0x09 && buff[5] == 0x8B ) {
			   printf( "order  = 3 \r\n" );
			   order = 3;
			
			}
		  }
	   }

	   if ( order == 3 ) {
		if ( verify_session_flag == 0 ) {
			 printf( "verify_session_flag == 0\r\n" );
			 G_ISO_TP.Request( UDS_PHY_AD_ID, ErreaseVIN_Sendout2, 6 ); /* 发送命令 */
			 verify_session_flag = 1;
		}
		
		if ( buff_read_flag == 1 ) {
			 buff_read_flag = 0;
			if ( buff[0] == 0x67 && buff[1] == 0x02 ) {
				order			= 0;
				modify_session_flag	= 0;
				secure_session_flag	= 0;
				verify_session_flag	= 0;
				modify_funsession_flag = 0 ;
				comm_funsession_flag  = 0 ;
				modify_session_flag1 = 0;
				start_secure_flag = 0; 
				UDS_SafeRequest_Status	= TRUEE;        /* 安全访问通过 */
				uds_connect_status = UDS_STATUS_CONNECTED;
			    uds_task_flag &= ~BLE_UDS_CMD_CONNECT;
				printf( "connet success \r\n" );
			}
		 }
	   }
        return UDS_SafeRequest_Status;
	}
  
   
}	
	
uint8_t uds_write_vin()	
{	
	if(!uds_is_connected()) {
       uds_task_flag |= BLE_UDS_CMD_CONNECT;
		return 0;
	}
	else {
	
	        uint8_t send_num = 0; 

		if ( Write_Vin_flag == 0 ) {
			memcpy( ErreaseVIN_Sendout3 + 3, SEND_VIN, sizeof(SEND_VIN) );          /* 将读取到的vin存入数组 */
			send_num = sizeof(SEND_VIN);
		//	printf( "send_num : %x \r\n ", send_num );
			G_ISO_TP.Request( UDS_PHY_AD_ID, ErreaseVIN_Sendout3, 3 + send_num );  /* 发送命令 */
			Write_Vin_flag	= 1;
			send_num	= 0;
		}


		if ( buff_read_flag == 1 ) {
			buff_read_flag = 0;
			if ( buff[0] == 0x6E && buff[1] == 0xF1 && buff[2] == 0x90 ) /* 建立会话 */
			{
				Write_Vin_flag		= 0;
				write_success_flag	= 1;
				printf( "write success\r\n" );
			    BT_SendAckMsg(BT_ACK_VIN_WRITE, 1);
                //g_BtCanSendFlag = 1;
			    uds_connect_status = UDS_STATUS_DISCONNECT;
			}
		}
	    return write_success_flag;
    }	
	
}	
	
uint8_t uds_send_sl_dtc()
{
	if(!uds_is_connected()) {
		uds_task_flag |= BLE_UDS_CMD_CONNECT;
	    return 0;
	}
	else {
	  
		if(Send_Sl_DTC_flag == 0) {
	      SL_DTC_Sendout[3] = SL_DTC ;
		  G_ISO_TP.Request( UDS_PHY_AD_ID, SL_DTC_Sendout, 4 ) ;
	      Send_Sl_DTC_flag = 1 ;
		}
	    
		if(buff_read_flag == 1) {
			 buff_read_flag = 0;
			if ( buff[0] == 0x6E && buff[1] == 0xF2 && buff[2] == 0x91 && buff[3] == SL_DTC) /* 建立会话 */
			{
				Send_Sl_DTC_flag		= 0;
				send_success_flag	    = 1;
				printf( "send success\r\n" );
			    uds_connect_status = UDS_STATUS_DISCONNECT;
			}
		}
	    return send_success_flag;
	
	}
}	


uint8_t   uds_clear_SL_DTC()
{
    if(!uds_is_connected()) {
		uds_task_flag |= BLE_UDS_CMD_CONNECT;
		return 0;
	}
	else {
      if(Clear_Sl_DTC_flag == 0) {
	      G_ISO_TP.Request( UDS_PHY_AD_ID, SL_DTC_Sendout1, 2 ) ;
	      Clear_Sl_DTC_flag = 1 ;
		}
	    
		if ( buff_read_flag == 1 ) {
			 buff_read_flag = 0;
			if ( buff[0] == 0x54 ) /* 建立会话 */
			{
				Clear_Sl_DTC_flag		= 0;
				clear_success_flag	    = 1;
				onClearSetTroubleCallback();
				printf( "clear success\r\n" );
			    uds_connect_status = UDS_STATUS_DISCONNECT;
			}
		}
	    return clear_success_flag;

    }
}

uint8_t uds_read_DTC()
{	
     uint8_t i =0;

	if(!uds_is_connected()) {
		uds_task_flag |= BLE_UDS_CMD_CONNECT;
		return 0;
	 }
    else {
      if(Read_DTC_flag == 0) {
	      G_ISO_TP.Request( UDS_PHY_AD_ID, READ_DTC, 2 ) ;
	      Read_DTC_flag = 1 ;
		}
	    
		if ( buff_read_flag == 1 ) {
			 buff_read_flag = 0;
			if ( buff[0] == 0x59 ) /* 建立会话 */
			{
				Read_DTC_Cnt = buff[1] ;
				for(i=0;i<=Read_DTC_Cnt;i++)
	            {
	              UDS_DTC_CODE[i] =buff[2+i] ;
	            }
				
				Read_DTC_flag		= 0;
				read_DTC_success_flag	    = 1;
				printf( "read DTC success\r\n" );
				onRequestHistoryTroubleCodeCallback(UDS_DTC_CODE, Read_DTC_Cnt);
			    uds_connect_status = UDS_STATUS_DISCONNECT;
			}
		}
	    return read_DTC_success_flag;

    }
}

uint8_t   uds_clear_DTC()
{
    if(!uds_is_connected()) {
		uds_task_flag |= BLE_UDS_CMD_CONNECT;
		return 0;
	}
	else {
      if(Clear_DTC_flag == 0) {
	      G_ISO_TP.Request( UDS_PHY_AD_ID, DTC_Sendout1, 2 ) ;
	      Clear_DTC_flag = 1 ;
		}
	    
		if ( buff_read_flag == 1 ) {
			 buff_read_flag = 0;
			if ( buff[0] == 0x54 ) /* 建立会话 */
			{
				Clear_DTC_flag		= 0;
				clear_DTC_success_flag	    = 1;
				onClearHistoryTroubleCallback();
				printf( "clear  DTC success\r\n" );
			    uds_connect_status = UDS_STATUS_DISCONNECT;
			}
		}
	    return clear_DTC_success_flag;
    }
}

uint8_t  uds_up_vcu()
{
    if(!uds_is_connected()) {
		uds_task_flag |= BLE_UDS_CMD_CONNECT;
		return 0;
	}
	else {
        switch(uds_status) {
		
		case UDS_UPG_ERASE_FLASH :
		     memcpy(&Request_Erase_Sendout[5], UpgProgAddr, 4);
		     memcpy(&Request_Erase_Sendout[9], UpgProgSize, 4);
		     G_ISO_TP.Request( UDS_PHY_AD_ID, Request_Erase_Sendout, 13 ) ;
		     uds_status = 0 ;
		     break;
		
		case UDS_UPG_DOWN_START :
		     switch(uds_upgrade_state) {   
		   
			 case uds_upgrade_start :
			      memcpy(&Request_download_Sendout[3], UpgProgAddr, 4);
		          memcpy(&Request_download_Sendout[7], UpgProgSize, 4);
		          G_ISO_TP.Request( UDS_PHY_AD_ID, Request_download_Sendout, 11 ) ;
			      uds_upgrade_state = 0 ;
			      break;
			 
			 case uds_upgrade_send :
			      G_ISO_TP.Request( UDS_PHY_AD_ID, Download_Data_Sendout, sendsize+2 ) ;
				  uds_upgrade_state = 0 ;
			      break;
			 
			 case uds_upgrade_end :
			      G_ISO_TP.Request( UDS_PHY_AD_ID, Download_End_Sendout, 2 ) ;
			      uds_upgrade_state = 0 ;
			      break;
			 
			 }
		     break;
			 
	    case UDS_UPG_DOWN_END :
		     memcpy(&UDS_CRC_Sendout[4], UpgCrcCode, 4);
		     G_ISO_TP.Request( UDS_PHY_AD_ID, UDS_CRC_Sendout, 8 ) ;
		     break;
	   
	    case UDS_UPG_VCU_RESET :
		     G_ISO_TP.Request( UDS_PHY_AD_ID, VCU_RESET_Sendout, 2 ) ;
		     break;
	  
	    case UDS_UPG_VCU_END1 :
			 delay_ms(1000);
		     G_ISO_TP.Request( UDS_FUN_AD_ID, Modify_Session_Sendout, 2 ) ;
		     break;
		
		case UDS_UPG_VCU_END2 :
		     G_ISO_TP.Request( UDS_FUN_AD_ID, COMM_Sendout1, 3 ) ;
		     uds_status = 0 ;
		     break;
		
		case UDS_UPG_VCU_END3 :
		     G_ISO_TP.Request( UDS_FUN_AD_ID, Modify_Session_Sendout2, 2 ) ;
		     break;
		
		}
		
	 if ( buff_read_flag == 1 )  {
		  buff_read_flag = 0;
	  
	      switch(buff[0]) {
			  
		  case  0x7F :  
			    if ( buff[1] == 0x31 && buff[2] == 0x78 ) {
					 UDS_upgrade_ack(UDS_UPGRADE_START, 1);
		        } 
		        break;
		  case  0x71 : 	  
			    if ( buff[1] == 0x01 && buff[2] == 0x03 && buff[3] == 0x03 ) {  
				     uds_status = UDS_UPG_DOWN_START;
				     UDS_upgrade_ack(UDS_UPGRADE_ERASE, 1);
				}
				 
				else if ( buff[1] == 0x01 && buff[2] == 0x02 && buff[3] == 0x02) {
			         uds_status = UDS_UPG_VCU_RESET ;
		        }
		        break;
		  
		  case  0x74 : 
		        if ( buff[1] == 0x20 && buff[2] == 0x01 && buff[3] == 0x02 ){
				     uds_upgrade_state = uds_upgrade_send ;
			    }
		        break;
		     
		  case  0x76 :
				if ( buff[1] == 0x01 ) {
				     uds_upgrade_state = uds_upgrade_end ;
			    }
		        break;
		  
		  case  0x77 :    
		        if ( buff[1] == 0x01 ) {
				     uds_upgrade_state = 0 ;
			         UDS_upgrade_ack(UDS_UPGRADE_CONTENT, 1);
				}
		        break;
		  case  0x51 :
			    if ( buff[1] == 0x01) {
			         //uds_status = UDS_UPG_VCU_END1 ;
					 UDS_upgrade_ack(UDS_UPGRADE_END, 1);
					 uds_status = 0 ;
		             uds_upgrade_success_flag = 1 ;
				     printf( "vcu upgrade success\r\n" );
				     uds_connect_status = UDS_STATUS_DISCONNECT;
					 
		        }
		        break;
		 
		  case  0x50 :
			    if ( buff[1] == 0x03 && buff[2] == 0x00 && buff[3] == 0x32 && buff[4] == 0x01 && buff[5] == 0xF4) {
			         uds_status = UDS_UPG_VCU_END2 ;
		        }
				
				else if ( buff[1] == 0x01 && buff[2] == 0x00 && buff[3] == 0x32 && buff[4] == 0x01 && buff[5] == 0xF4) {
			         uds_status = 0 ;
		             uds_upgrade_success_flag = 1 ;
				     printf( "vcu upgrade success\r\n" );
				     uds_connect_status = UDS_STATUS_DISCONNECT;
				     
				}
		        break; 
		  
		  case  0x68 :
			    if ( buff[1] == 0x00) {
			         uds_status = UDS_UPG_VCU_END3 ;
		        }
		        break;
			  
		  }   
			  
	  }
	 return  uds_upgrade_success_flag  ;
  }	  
}	  



void Time_Watch( void )                                                                         /* 时间信号 */
{
	Time.Second++;
	if ( Time.Second >= 60 )
	{
		Time.Second = 0;
		Time.Min++;
	}
	if ( Time.Min >= 60 )
	{
		Time.Min = 0;
		Time.Hour++;
	}
	if ( Time.Hour >= 24 )
	{
		Time.Hour = 0;
		Time.Day++;
	}
	if ( Time.Day > 30 )
	{
		Time.Day = 0;
		Time.Month++;
	}
	if ( Time.Month > 12 )
	{
		Time.Month = 0;
		Time.Year++;
	}
}


