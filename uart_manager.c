#include "uart_manager.h"

#define UART_BT_CHANNEL			UART1_CHANNEL
#define UART_DEBUG_CHANNEL		UART2_CHANNEL

#define UART_BT_RX_BUFF_MAX_SIZE		1024
#define UART_BT_TX_BUFF_MAX_SIZE		512

void UART_BT_Recv(UART_TypeDef *p);

void UART_DEBUG_Init(u32 bps)
{
	UART_TypeDef *p = &UART_TypeDefs[UART_DEBUG_CHANNEL];
	
	p->baud = bps;
	UART_GPIO_Config(p);
	UART_Baud_Config(p);
}

	static u8 rxBuf[UART_BT_RX_BUFF_MAX_SIZE];
	static u8 txBuf[UART_BT_TX_BUFF_MAX_SIZE];
void UART_BT_Init(u32 bps)
{
	UART_TypeDef *p = &UART_TypeDefs[UART_BT_CHANNEL];

	p->baud = bps;
	
	UART_GPIO_Config(p);
	UART_Baud_Config(p);
	
	UART_IRQ_Config(p);
	
	UART_Buff_Config(p, rxBuf, sizeof(rxBuf), txBuf, sizeof(txBuf));
	UART_DMA_RX_Config(p);
	//UART_DMA_TX_Config(p);
}

void UART_SendToBt(u8 *data, u16 len) 
{
	UART_TypeDef *p = &UART_TypeDefs[UART_BT_CHANNEL];
	//UART_DMASend(p, data, len);
	UART_Send(p, data, len);
}

void UART_Clear_BtBuff()
{
	UART_TypeDef *p = &UART_TypeDefs[UART_BT_CHANNEL];
	memset(p->buff.rx_buf, 0, p->buff.rx_buf_max);
}

u16 UART_Get_BtBuff(u8 *data)
{
	return UART_GetData(&UART_TypeDefs[UART_BT_CHANNEL], data, NULL);
}





















