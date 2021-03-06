/*
 * debug.h
 *
 *  Created on: 2016-7-1
 *      Author: Administrator
 */

#ifndef DEBUG_H_
#define DEBUG_H_
#define	DEBUG_DRIVE_LEVEL

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define NIC_TRACE_LEVEL			0
#define TRACE_LEVEL_DEBUG	1



//#define DEBUG_SWITCH

#ifdef    DEBUG_SWITCH
#define TRACE_INFO(fmt,args...) printf(fmt, ##args)
#else
#define TRACE_INFO(fmt,args...) /*do nothing */
#endif

//#define 	DEBUG_PRINT
#ifdef    DEBUG_PRINT
#define TRACE_DEBUG(fmt,args...) printf(fmt, ##args)
#else
#define TRACE_DEBUG(fmt,args...) /*do nothing */
#endif





#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"


typedef struct {
	uint32_t		irq_count[2];
	uint32_t		irq_handle_count[2];
	uint32_t		event_count[2];
	uint32_t		EventHandler[2];
	uint32_t		irq_set_count;
	uint32_t		linkdown_count[2];
	uint32_t		enc624_recv_abort[2];

	uint32_t		send_count[2];
	uint32_t		pbuf_malloc_fail[2];
	uint32_t		recv_count[2];
	uint32_t		recv_err_seq[2];
	uint32_t		send_busy_count[2];


	uint32_t		enc624_test_thread_run[2];

	uint32_t		pbuf_free_local;
}Sys_deginfo;



#define LWIP_DEBUGF(debug, message) TRACE_DEBUG(message)


#define LWIP_ASSERT(message, assertion) do { if(!(assertion)) \
		TRACE_DEBUG(message); \
		assert( assertion);} while(0)

#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
		LWIP_ASSERT(message,expression); handler;}} while(0)

extern Sys_deginfo		Dubug_info;
//#define DEBUG_ONLY_GPIO_INIT			//调试GPIO配置的时候出现SIGBUS的问题
#endif /* DEBUG_H_ */
