#include <stdio.h>
#include <string.h>
#include "ISOTP.h"
#include "type.h"
#include "can.h"
#include "uds.h"
//#include "timer.h"

//uint32_t callback =0;
uint8_t UDS_Data[8];
uint8_t reeultt=0;
uint8_t buff_read_flag=0; //buff_read_flag
uint8_t modify_session_flag=0;
uint8_t secure_session_flag=0;
uint8_t verify_session_flag=0;
uint8_t modify_funsession_flag=0;
uint8_t comm_funsession_flag=0;
uint8_t modify_session_flag1=0;
uint8_t request_download_flag=0;
uint8_t Write_Vin_flag =0;
uint8_t Read_Vin_flag  =0;
uint8_t write_success_flag = 0;
uint8_t read_success_flag=0;
uint8_t start_secure_flag = 0;
extern uint8_t VIN_Receive_Respond_Flag;
extern uint8_t ErreaseVIN_Response[8];
extern uint32_t UDS_RESPOND_ID;
extern uint32_t g_SysTicks;
extern uint8_t order ;
/*****************
 * net work
 */
void CallBack_Confirm(uint32_t ID,N_Result Result)//发送完成结果回掉
{		
	//callback=99;
}

 uint8_t buff[4095]={0} ;
	
uds_recv_callback_fun_t uds_recv_callback_fun = NULL;
void UDS_setRecvCallback(uds_recv_callback_fun_t fun)
{
	uds_recv_callback_fun = fun;
}

/*************
 * 首帧接受回调
 */
void CallBack_FF_Indication(uint32_t ID,uint32_t Length,uint8_t**HeadData)//接收到首帧回掉
{
	//printf("dao CallBack FF Indication \r\n");
	*HeadData=buff;
	//reeultt = Length;
}
/****************
 * 接受完成回调
 */
void CallBack_Indication(uint32_t ID,uint8_t* MessageData,uint32_t Length,N_Result Result)
{
//	printf("CallBack Indication \r\n");  
	uint8_t buf;
	
	    
	if(Result==N_OK)//N_OK
	{	
//	 printf("Result is OK \r\n");  
	 //printf("Length = %d \r\n",Length);  
		for(buf=0;buf<Length;buf++)
		{
			buff[buf]=MessageData[buf];//接受完成 数据直接储存在buff[]数组中
		}
		//callback=Length;
		
		buff_read_flag=1;

//	    printf("buff_read_flag:%d\r\n",buff_read_flag);   
	    buf=(uint8_t)ID;
	    ID&=0xffffff00;
	    ID|=(uint8_t)(ID>>8);
	    ID&=0xffff00ff;
	    ID|=buf<<8;
		UDS_RESPOND_ID= ID;	//流控回复ID 把接受到的ID后两位地址替换	
		reeultt++;
		
//	    if(uds_recv_callback_fun != NULL) {
//	    uds_recv_t udsRecv;
//	    udsRecv.server = 
//	    uds_recv_callback_fun();
//	    }
	}

}
/**************************
 * 数据链路
 */
void DLConfirm(uint32_t ID)
{
	G_ISO_DL.Confirm(ID,OK);
}

/*******
 * 接受结果回调
 */
 uint16_t Send_cnt=0;
void DLIndication(uint32_t ID,uint32_t*Data,uint8_t Length)
{
	G_ISO_DL.Indication(ID,Data,Length);
}


void CanUpg_RecvData(uint32_t ID,uint32_t*Data,uint8_t Length)
{
	
	switch (ID) {
		case 0xf5:
        if(uds_task_flag & BLE_UDS_CMD_UP_VCU){
			//printf("Data[0]:%x  ,   Data[1]:%x \r\n",Data[0],Data[1]);
            if (Data[0] == 0x01010201 && Data[1] == 0x00000100) {
			 printf("0xf5 callback \r\n");
			 start_secure_flag = 1;
			}
        }
        break;
    }
}
	
void SendData(uint32_t ID,uint32_t*Data,uint8_t Length)
{   
	TYPE_Can canMsg;
	uint8_t *inputData =(uint8_t *)Data;
	Send_cnt++;
	 
	//printf("send data ！\r\n");
	//printf("data ptr:%x,content:%02x %02x %02x %02x %02x %02x %02x %02x\r\n", (uint32_t)Data, 
	//inputData[0],inputData[1],inputData[2],inputData[3],inputData[4],inputData[5],inputData[6],inputData[7]);
	
	memcpy(canMsg.data,inputData,(Length>8?8:Length));
	
	canMsg.ch = CAN_CH2;
	canMsg.id = ID;
	canMsg.dlc = Length;
//	printf("memcpy success ！\r\n");
	CAN_SendExtMsg(&canMsg);//回复流控
  
	//printf("ID = %08x  UDSdata = %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n", ID ,
    //Data[0],Data[1],Data[2],Data[3],Data[4],Data[5],Data[6],Data[7]);  
	DLConfirm(0x18dad0f9);
}

int ISOTP_Init(void) 
{ 
	G_ISO_DL.Request=SendData;

	G_ISO_TP.FF_Indication=CallBack_FF_Indication;
	G_ISO_TP.Confirm=CallBack_Confirm;
	G_ISO_TP.Indication=CallBack_Indication;
	G_ISO_TP.nowTime=(uint32_t*)(&g_SysTicks);//传入系统时间，1MS变量

	return 0;
}
