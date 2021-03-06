/*
 * user_main.h
 *
 *  Created on: 2019��4��11��
 *      Author: pengfeizhao
 */

#ifndef APP_USER_USER_MAIN_H_
#define APP_USER_USER_MAIN_H_

#define UDP_PORT 777
#define TCP_PORT 888

#include"osapi.h"

#include"user_interface.h"
#include"espconn.h"
#include"mem.h"
#include "udp_config.h"
#include "server.h"

ETSTimer connect_timer;
ETSTimer test_timer;
ETSTimer station_check;

struct espconn user_udp_espconn;
struct espconn user_tcp_conn;
#endif /* APP_USER_USER_MAIN_H_ */
