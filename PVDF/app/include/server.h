/*
 * server.h
 *
 *  Created on: 2019Äê6ÔÂ12ÈÕ
 *      Author: pengfeizhao
 */

#ifndef APP_USER_SERVER_H_
#define APP_USER_SERVER_H_
#include "user_main.h"

struct command
{
	char MSgId;
	char MsgObj[7];
	char CON_STATUS;
	char ssid[32];
	char password[64];
};
void  ICACHE_FLASH_ATTR server_init(int port);
void ICACHE_FLASH_ATTR wifi_connected(void *arg);
int ICACHE_FLASH_ATTR value_as_int(char *key_value,char length);
uint16_t ICACHE_FLASH_ATTR comtype_parse(char* pdata,char* key);
#endif /* APP_USER_SERVER_H_ */
