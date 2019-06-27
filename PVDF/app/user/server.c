/*
 * server.c
 *
 *  Created on: 2019年6月12日
 *      Author: pengfeizhao
 */
#include "server.h"
#include "user_json.h"

struct bss_info*bss_link;

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

LOCAL int ICACHE_FLASH_ATTR
device_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
	int type;
	char buffer[32];
	struct station_config stationConf;

	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if (jsonparse_strcmp_value(parser, "ssid") == 0) {

				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser,buffer,sizeof(buffer));
				os_printf("%s", buffer);
				os_memcpy(&stationConf.ssid,buffer,32);
					 os_memcpy(&stationConf.password,"13166982258",64);
					 wifi_station_set_config_current(&stationConf);
					 wifi_station_connect();
					 os_timer_setfn(&connect_timer,wifi_connected,NULL);
					 os_timer_arm(&connect_timer,2000,NULL);
			 }
		   }
		}
	return 0;
}

LOCAL struct jsontree_callback wifi_station_callback =
JSONTREE_CALLBACK(wifi_scan_get, device_parse);

JSONTREE_OBJECT(ssid_tree, JSONTREE_PAIR("ssid", &wifi_station_callback),
		JSONTREE_PAIR("rssi", &wifi_station_callback));


void ICACHE_FLASH_ATTR server_recvcb(void*arg, char*pdata, unsigned short len) {
	struct espconn *pesp_conn = arg;
	struct jsontree_context js;
	remot_info *premot = NULL;

	jsontree_setup(&js, (struct jsontree_value *) &ssid_tree, json_putchar);
	json_parse(&js, pdata);

	/*os_printf("local_port3:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->local_port
	 ,IP2STR(pesp_conn->proto.tcp->remote_ip));*/

	espconn_send(pesp_conn, pdata, os_strlen(pdata));
	//os_printf("%s\r\n", pdata);

}
void ICACHE_FLASH_ATTR server_sentcb(void*arg) {
	os_printf("发送成功\r\n");
}
void ICACHE_FLASH_ATTR server_disconcb(void*arg) {
	struct ip_info ap_ip;
	os_printf("断开连接\r\n");

}
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status) {

	uint8 DeviceBuffer[1000] = "wifi list:\r\n";
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


	}
}
void ICACHE_FLASH_ATTR server_listen(void *arg) {

	LOCAL connect_status = 0;

	if (connect_status == 0) {
		connect_status++;
		os_printf("连接设备状态idle");
	} else {
		os_printf("连接设备状态busy");
	}
	wifi_station_scan(NULL, scan_done);
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

