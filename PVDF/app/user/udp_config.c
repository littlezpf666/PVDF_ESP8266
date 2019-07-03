/*
 * udp_config.c
 *
 *  Created on: 2019年6月16日
 *      Author: pengfeizhao
 */
#include "udp_config.h"
#include "user_json.h"
extern struct command rec_comm;
int ICACHE_FLASH_ATTR
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
	if (os_strncmp(path, "model", 5) == 0) {
		jsontree_write_int(js_ctx, wifi_get_opmode());
	}
	if (os_strncmp(path, "device_name", 11) == 0) {
		jsontree_write_string(js_ctx, "PVDF");
	}
	if (os_strncmp(path, "battery_power", 13) == 0) {
		jsontree_write_int(js_ctx, 30);
	}
	if (os_strncmp(path, "MsgStatus", 9) == 0) {
		jsontree_write_string(js_ctx, "True");
	}

	return 0;
}
LOCAL struct jsontree_callback TXD_1_callback =
JSONTREE_CALLBACK(TXD_1_get, NULL);

JSONTREE_OBJECT(DEVICE_INFO, JSONTREE_PAIR("model", &TXD_1_callback),
		JSONTREE_PAIR("device_name", &TXD_1_callback),
		JSONTREE_PAIR("battery_power", &TXD_1_callback),
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
		remot_info	*premot	=	NULL;
		char *pbuf = NULL;
		pbuf = (char *) os_zalloc(100);

		jsontree_setup(&js, (struct jsontree_value *) &rece_1_tree, json_putchar);
		json_parse(&js, pdata);
		if(!(os_strcmp(rec_comm.MsgObj,"Mobile")))
		{
			memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
			json_ws_send((struct jsontree_value *) &TXD_1_tree, "decvice_inform",pbuf);
	    }
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
