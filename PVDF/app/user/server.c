/*
 * server.c
 *
 *  Created on: 2019年6月12日
 *      Author: pengfeizhao
 */
#include "server.h"
#include "user_json.h"

struct bss_info*bss_link;
LOCAL connect_status = 0;
struct command rec_comm;
void ICACHE_FLASH_ATTR wifi_connected(void *arg){
	static uint8 count=0;
	uint8 status;
	uint8 bssid[6];

	struct station_info*st_info;
	struct	ip_info	ap_ip;

	os_timer_disarm(&connect_timer);
	count++;

	status=wifi_station_get_connect_status();
	if(status==STATION_GOT_IP){//	STATION_GOT_IP
		espconn_sent(&user_tcp_conn,"success",strlen("success"));
		return;
	}
	else{
		if(count>=7)
		{
			count=0;
			os_printf("None station connect !\n");
		}
	}
	os_timer_arm(&connect_timer,2000,NULL);
}

/*****************************CJSON wifi_scan***********************************/
LOCAL int ICACHE_FLASH_ATTR
wifi_scan_get(struct jsontree_context *js_ctx) {
	//uint8_t MAC[18];
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

	if (os_strncmp(path, "ssid", 4) == 0) {
		jsontree_write_string(js_ctx, bss_link->ssid);
	}
	if (os_strncmp(path, "rssi", 4) == 0) {
			jsontree_write_int(js_ctx, bss_link->rssi);
		}
	/*if (os_strncmp(path, "MAC", 3) == 0) {
		os_sprintf(MACSTR,MAC2STR(bss_link->bssid));
				jsontree_write_string(js_ctx, MAC);
		}*/

	return 0;
}

/*LOCAL int ICACHE_FLASH_ATTR
wifi_conn_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
	int type;
	char ssid[32];
	char password[64];
	struct station_config stationConf;

	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if (jsonparse_strcmp_value(parser, "ssid") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser,ssid,sizeof(ssid));
                os_printf("ssid:%s\r\n",ssid);
				os_memcpy(&stationConf.ssid,ssid,32);
			 }
			else if (jsonparse_strcmp_value(parser, "password") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser, password, sizeof(password));
				os_printf("password:%s\r\n", password);
				os_memcpy(&stationConf.password, password, 64);

				wifi_station_set_config_current(&stationConf);
				wifi_station_connect();
				os_timer_setfn(&connect_timer, wifi_connected, NULL);
				os_timer_arm(&connect_timer, 2000, NULL);
			 }

		   }
		}
	return 0;
}*/

LOCAL struct jsontree_callback wifi_scan_callback =
JSONTREE_CALLBACK(wifi_scan_get, NULL);

JSONTREE_OBJECT(ssid_tree, JSONTREE_PAIR("ssid", &wifi_scan_callback),
		JSONTREE_PAIR("rssi", &wifi_scan_callback));
LOCAL int ICACHE_FLASH_ATTR
rece_1245_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
	int type;

	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if (jsonparse_strcmp_value(parser, "MsgObj") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser,rec_comm.MsgObj,sizeof(rec_comm.MsgObj));
                os_printf("MsgObj:%s\r\n",rec_comm.MsgObj);
			 }
		   }
		}
	return 0;
}
LOCAL struct jsontree_callback rece_1245_callback =
JSONTREE_CALLBACK(NULL, rece_1245_parse);

JSONTREE_OBJECT(rece_1245_tree, JSONTREE_PAIR("MsgId", &rece_1245_callback),
		JSONTREE_PAIR("MsgObj", &rece_1245_callback),
		JSONTREE_PAIR("MsgStatus", &rece_1245_callback),
		);

/*LOCAL struct jsontree_callback wifi_conn_callback =
JSONTREE_CALLBACK(NULL, wifi_conn_parse);

JSONTREE_OBJECT(conn_tree, JSONTREE_PAIR("ssid", &wifi_conn_callback),
		JSONTREE_PAIR("password", &wifi_conn_callback));*/
/*****************************扫描回调***********************************/
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status) {

	uint8 DeviceBuffer[2000]= "wifi list:\r\n";
	remot_info *premot = NULL;
	char *pbuf = NULL;
	pbuf = (char *) os_zalloc(60);

	if (status == OK) {
		bss_link = (struct bss_info*) arg;
		//bss_link = bss_link->next.stqe_next;
		while (bss_link != NULL ) {
			json_ws_send((struct jsontree_value *) &ssid_tree, "ssid_list",pbuf);
			os_strcat(DeviceBuffer, pbuf);
			os_memset(pbuf, 0, 60);
			bss_link = bss_link->next.stqe_next;
		}

		os_free(pbuf);
		/*一般情况，请在前一包数据发送成功，进入 espconn_sent_callback 后，再调用 espconn_send 发送下一包数据。*/
		espconn_send(&user_tcp_conn, DeviceBuffer, strlen(DeviceBuffer));
		os_free(DeviceBuffer);

	}
}
int ICACHE_FLASH_ATTR value_as_int(char *key_value,char length)
{
	uint8 i,j;
	uint16_t order=1;
	uint16_t key_integer=0;
	for(i=0;i<length;i++)
	{
		/*将字符转化为整形*/
		for(j=0;j<(length-i-1);j++)
		{
			order=order*10;
		}
		key_integer+=((*(key_value+i)-48)*order) ;
		order=1;
	}

	return key_integer;
}
uint16_t ICACHE_FLASH_ATTR comtype_parse(char* pdata,char* key)
{
	uint8 length,i;
	uint16_t key_integer=0;

	char *parse_begin,*parse_end;
	/*不要再此定义数组因为除出了函数内存就释放掉了，也可定义字符串但字符串是常量，不会被释放掉，
	 而且定义空间大小很麻烦，因此选用动态产生释放内存的方法*/
	char *key_value = NULL;
	key_value = (char *) os_zalloc(5);
	parse_begin=(char *)os_strstr(pdata,key);

	if (parse_begin != NULL){
		parse_begin+=(strlen(key)+2);
		parse_end=os_strchr(parse_begin,',');
		length=parse_end-parse_begin;
		os_printf("length:%d\r\n",length);
		for(i=0;i<length;i++)
		{
		key_value[i]=*(parse_begin+i);
		}
		key_integer=value_as_int(key_value,length);
		os_free(key_value);
		os_printf("comm_type:%d\r\n",key_integer);
	}
	return key_integer;
}
void ICACHE_FLASH_ATTR server_recvcb(void*arg, char*pdata, unsigned short len) {
		struct espconn *pesp_conn = arg;
		struct jsontree_context js;
		remot_info *premot = NULL;
		char* key="MsgId";
		char* key_value="scan";

		switch(comtype_parse(pdata,key))
		{
		case 1:
		case 2:
		case 4:
		case 5:
			jsontree_setup(&js, (struct jsontree_value *) &rece_1245_tree, json_putchar);
			json_parse(&js, pdata);
			if(!(os_strcmp(rec_comm.MsgObj,key_value)))
				{
					wifi_station_scan(NULL, scan_done);
				}
			break;
		case 3:
			break;
		}
	espconn_send(pesp_conn, pdata, os_strlen(pdata));
	/*os_printf("local_port3:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->local_port
	 ,IP2STR(pesp_conn->proto.tcp->remote_ip));*/
}
void ICACHE_FLASH_ATTR server_sentcb(void*arg) {
	os_printf("发送成功\r\n");
}
void ICACHE_FLASH_ATTR server_disconcb(void*arg) {
	struct ip_info ap_ip;
	connect_status--;
	os_printf("断开连接\r\n");

}

void ICACHE_FLASH_ATTR server_listen(void *arg) {

	if (connect_status == 0) {
		connect_status++;
		os_printf("连接设备状态idle\r\n");
	} else {
		os_printf("连接设备状态busy\r\n");
	}

	espconn_regist_recvcb((struct espconn*) arg, server_recvcb);
	espconn_regist_sentcb((struct espconn*) arg, server_sentcb);
	espconn_regist_disconcb((struct espconn*) arg, server_disconcb);
}
void ICACHE_FLASH_ATTR server_reconn(void *arg, sint8 err) {
	os_printf("连接错误，错误代码为%d\r\n", err);
}
void ICACHE_FLASH_ATTR server_init(int port) {

	//espconn参数配置
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	user_tcp_conn.proto.tcp->local_port = port;

	//注册连接和重连接回调函数
	espconn_regist_connectcb(&user_tcp_conn, server_listen);
	espconn_regist_reconcb(&user_tcp_conn, server_reconn);

	//启动连接
	espconn_accept(&user_tcp_conn);
	espconn_regist_time(&user_tcp_conn, 0, 0); //超时断开连接时间，这个必须要设置否则会默认10s左右自动断开，写0永久连接

}

