/*
 * osAbstraction.h
 *
 *  Created on: 2016-7-4
 *      Author: Administrator
 */

#ifndef OSABSTRACTION_H_
#define OSABSTRACTION_H_
#include "error_head.h"


err_t osSetEvent( void *event);
int osSetEventFromIsr( void *event);

#endif /* OSABSTRACTION_H_ */