/*
 * socket_esxception.cpp
 *
 *  Created on: 21 мар. 2019 г.
 *      Author: res
 */

#include "socket_exception.h"

#define XX(num, name, helpstr)		helpstr,
static const char * socket_exception_hlp[] = {
	SOCKET_EXCEPTION_MAP(XX)
};
#undef XX

const char * socket_exception_str(enum socket_exception_e se_id)
{
	return socket_exception_hlp[se_id - 1];
}
