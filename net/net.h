/*
 * net_method.h
 *
 *  Created on: 2016-7-4
 *      Author: Administrator
 */

#ifndef NET_H_
#define NET_H_
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "error_head.h"
#include <sys/types.h>
#include "lw_oopc.h"
#include "am335x_gpmc.h"
#include "am335x_gpio.h"
#include <pthread.h>
#include <atomic.h>


#define MAC_LEN		6
#define RX_EVENT	1
#define TX_EVENT	2

#define	PBUFFLAG_TRASH		0		///< 已经处理完成，无用的数据
#define	PBUFFLAG_UNPROCESS	1		///< 未处理的数据
#define	PBUFFLAG_DEALING	2		///< 正在处理队列中的数据

#define		ISR_TX_EVENT 		( 1 << 0)
#define		ISR_RX_EVENT 		( 1 << 1)
#define		ISR_LINK_STATUS_CHG ( 1 << 2)
#define		ISR_RECV_PACKET 	( 1 << 3)
#define		ISR_TRAN_COMPLETE 	( 1 << 4)
#define		ISR_PROCESSING 		( 1 << 5)			//
#define		ISR_RECV_ABORT		( 1 << 6)

#define		ISR_NONE ( 1 << 31)

typedef struct
{
	char addr[6];
}MacAddr;

typedef struct
{
	uint16_t w[3];
}MacAddr_u16;

typedef struct
{
	uint_t		chunkCount;
	uint_t		len;
	uint8_t		*data;

}NetBuffer;

typedef struct
{
	char					*name;

	uint8_t					*ethFrame;
	void 					*nicContext;

	unsigned int			macFilterSize;
	MacAddr_u16					*macFilter;

	MacAddr_u16				*macAddr;

	Drive_Gpmc				*busDriver;
	Drive_Gpio				*extIntDriver;

	bool					speed100;
	bool					fullDuplex;
	bool					linkState;
	uint8_t					instance;

	int 					nicRxEvent;
	int						nicTxEvent;

	pthread_t				isr_tid;
	pthread_t				send_tid;
	volatile uint32_t		isr_status;
	void					*hl_netif;		//上一层的网络接口
	struct pbuf				*rxUnprocessed;		///< 接收但没来得及处理的数据.这个元素只能在程序中一个位置访问。这样来避免上锁操作
	struct pbuf				*rxpbuf;


}NetInterface;

typedef struct {
	void *next;

}list_node;

err_t macCompAddr( void *mac1, void *mac2);
err_t nicNotifyLinkChange( NetInterface *Inet );
err_t nicProcessPacket( NetInterface * Inet, uint8_t *frame, int len);

int netBufferGetLength( const NetBuffer *net_buf);
err_t insert_node_to_listtail(void **listhead, void *newnode);
#endif /* NET_METHOD_H_ */
