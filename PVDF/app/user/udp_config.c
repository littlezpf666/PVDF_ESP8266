/*
 * udp_config.c
 *
 *  Created on: 2019年6月16日
 *      Author: pengfeizhao
 */
#include "udp_config.h"
#include "user_json.h"
extern struct command rec_comm;

LOCAL int ICACHE_FLASH_ATTR
RXD_1_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
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

LOCAL struct jsontree_callback rece_1_callback =
JSONTREE_CALLBACK(NULL, RXD_1_parse);

JSONTREE_OBJECT(rece_1_tree, JSONTREE_PAIR("MsgId", &rece_1_callback),
		JSONTREE_PAIR("MsgObj", &rece_1_callback),
		JSONTREE_PAIR("MsgStatus", &rece_1_callback),
		);

int ICACHE_FLASH_ATTR
TXD_1_get(struct jsontree_context *js_ctx) {

	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

	if (os_strncmp(path, "MsgId", 5) == 0) {
		jsontree_write_int(js_ctx, 1);
	}
	if (os_strncmp(path, "NetworkMode", 5) == 0) {


		jsontree_write_int(js_ctx, rec_comm.opmode);

	}
	if (os_strncmp(path, "Name", 11) == 0) {
		jsontree_write_string(js_ctx, "PVDF");
	}
	if (os_strncmp(path, "Power", 13) == 0) {


			jsontree_write_int(js_ctx, 35);
		}
	if (os_strncmp(path, "Ip", 9) == 0) {

		jsontree_write_string(js_ctx, rec_comm.device_ip);

	}
	if (os_strncmp(path, "MsgStatus", 9) == 0) {
		jsontree_write_string(js_ctx, "True");
	}

	return 0;
}
LOCAL struct jsontree_callback TXD_1_callback =
JSONTREE_CALLBACK(TXD_1_get, NULL);

JSONTREE_OBJECT(DEVICE_INFO, JSONTREE_PAIR("NetworkMode", &TXD_1_callback),
		JSONTREE_PAIR("Name", &TXD_1_callback),
		JSONTREE_PAIR("Power", &TXD_1_callback),

		JSONTREE_PAIR("Ip", &TXD_1_callback)


		);
JSONTREE_OBJECT(TXD_1_tree, JSONTREE_PAIR("MsgId", &TXD_1_callback),
		JSONTREE_PAIR("MsgObj", &DEVICE_INFO),
		JSONTREE_PAIR("MsgStatus", &TXD_1_callback),
		);
void ICACHE_FLASH_ATTR user_udp_send(void){
	struct	ip_info	ap_ip;
	char hwaddr[6];
	char DeviceBuffer[40]={0};
	char IPBuffer[10]={0};

	espconn_sent(&user_udp_espconn,"success",strlen("success"));
}
void ICACHE_FLASH_ATTR user_ubp_sent_cb(void*arg){
	os_printf("UDP发送完毕!\r\n");
}
void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg,char * pdata,unsigned short len)
{
	    struct jsontree_context js;
		struct	espconn	*pesp_conn	=	arg;
		struct	ip_info	info;
		remot_info	*premot	=	NULL;


		char *pbuf = NULL;
		pbuf = (char *) os_zalloc(170);//要开辟足够多的空间

		jsontree_setup(&js, (struct jsontree_value *) &rece_1_tree, json_putchar);
		json_parse(&js, pdata);
		if(!(os_strcmp(rec_comm.MsgObj,"Mobile")))
		{
			memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
            /********************获取本地IP,与WIFI模式******************************/
			//在station_softap下ESP8266有两个IP（目前认为相当于两台设备）,
			//一个是station的，与其同一局域网的station要访问他，要用这个IP
			//另一个是softap的，连接到其热点的station要访问他，要用这个IP
			rec_comm.opmode=wifi_get_opmode();
			if(rec_comm.opmode==3)
			wifi_get_ip_info(SOFTAP_IF,&info);
			else if(rec_comm.opmode==1)
			wifi_get_ip_info(STATION_IF,&info);
			rec_comm.device_ip=(char *)os_malloc(20);
			os_sprintf(rec_comm.device_ip,IPSTR,IP2STR(&info.ip));

			json_ws_send((struct jsontree_value *) &TXD_1_tree, "decvice_inform",pbuf);
			os_free(rec_comm.device_ip);

		if (espconn_get_connection_info(pesp_conn,&premot,0)==ESPCONN_OK){
			pesp_conn->proto.udp->remote_port  = premot->remote_port;
			pesp_conn->proto.udp->remote_ip[0] = premot->remote_ip[0];
			pesp_conn->proto.udp->remote_ip[1] = premot->remote_ip[1];
			pesp_conn->proto.udp->remote_ip[2] = premot->remote_ip[2];
			pesp_conn->proto.udp->remote_ip[3] = premot->remote_ip[3];

			espconn_sent(pesp_conn,pbuf,strlen(pbuf));
			os_free(pbuf);
		}
		}
}
void user_udp_init(int local_port)
{
	user_udp_espconn.type=ESPCONN_UDP;
	user_udp_espconn.proto.udp=(esp_udp*)os_zalloc(sizeof(esp_udp));
	user_udp_espconn.proto.udp->local_port=local_port;
	//user_udp_espconn.proto.udp->remote_port=8888;
	//const char udp_remote_ip[4]={192,168,43,154};
	//os_memcpy(&user_udp_espconn.proto.udp->remote_ip,udp_remote_ip,4);
	espconn_regist_recvcb(&user_udp_espconn,user_udp_recv_cb);
	espconn_regist_sentcb(&user_udp_espconn,user_ubp_sent_cb);
	espconn_create(&user_udp_espconn);
}
