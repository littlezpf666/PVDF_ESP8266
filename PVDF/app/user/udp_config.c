/*
 * udp_config.c
 *
 *  Created on: 2019��6��16��
 *      Author: pengfeizhao
 */
#include "udp_config.h"
#include "user_json.h"
extern struct command rec_comm;
<<<<<<< HEAD
LOCAL int ICACHE_FLASH_ATTR
=======
int ICACHE_FLASH_ATTR
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
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


<<<<<<< HEAD
LOCAL int ICACHE_FLASH_ATTR
TXD_1_get(struct jsontree_context *js_ctx) {

=======
int ICACHE_FLASH_ATTR
TXD_1_get(struct jsontree_context *js_ctx) {
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

	if (os_strncmp(path, "MsgId", 5) == 0) {
		jsontree_write_int(js_ctx, 1);
	}
	if (os_strncmp(path, "model", 5) == 0) {
<<<<<<< HEAD

		jsontree_write_int(js_ctx, rec_comm.opmode);
=======
		jsontree_write_int(js_ctx, wifi_get_opmode());
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
	}
	if (os_strncmp(path, "device_name", 11) == 0) {
		jsontree_write_string(js_ctx, "PVDF");
	}
	if (os_strncmp(path, "battery_power", 13) == 0) {
<<<<<<< HEAD

			jsontree_write_int(js_ctx, 35);
		}
	if (os_strncmp(path, "device_ip", 9) == 0) {

		jsontree_write_string(js_ctx, rec_comm.device_ip);
=======
		jsontree_write_int(js_ctx, 30);
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
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
<<<<<<< HEAD
		JSONTREE_PAIR("device_ip", &TXD_1_callback)
=======
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
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
	os_printf("UDP�������!\r\n");
}
void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg,char * pdata,unsigned short len)
{
	    struct jsontree_context js;
		struct	espconn	*pesp_conn	=	arg;
		struct	ip_info	info;
		remot_info	*premot	=	NULL;
<<<<<<< HEAD

		char *pbuf = NULL;
		pbuf = (char *) os_zalloc(170);//Ҫ�����㹻��Ŀռ�

		jsontree_setup(&js, (struct jsontree_value *) &rece_1_tree, json_putchar);
		json_parse(&js, pdata);
		if(!(os_strcmp(rec_comm.MsgObj,"Mobile")))
		{
			memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
            /********************��ȡ����IP,��WIFIģʽ******************************/
			//��station_softap��ESP8266������IP��Ŀǰ��Ϊ�൱����̨�豸��,
			//һ����station�ģ�����ͬһ��������stationҪ��������Ҫ�����IP
			//��һ����softap�ģ����ӵ����ȵ��stationҪ��������Ҫ�����IP
			rec_comm.opmode=wifi_get_opmode();
			if(rec_comm.opmode==3)
			wifi_get_ip_info(SOFTAP_IF,&info);
			else if(rec_comm.opmode==1)
			wifi_get_ip_info(STATION_IF,&info);
			rec_comm.device_ip=(char *)os_malloc(20);
			os_sprintf(rec_comm.device_ip,IPSTR,IP2STR(&info.ip));

			json_ws_send((struct jsontree_value *) &TXD_1_tree, "decvice_inform",pbuf);
			os_free(rec_comm.device_ip);
=======
		char *pbuf = NULL;
		pbuf = (char *) os_zalloc(100);

		jsontree_setup(&js, (struct jsontree_value *) &rece_1_tree, json_putchar);
		json_parse(&js, pdata);
		if(!(os_strcmp(rec_comm.MsgObj,"Mobile")))
		{
			memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
			json_ws_send((struct jsontree_value *) &TXD_1_tree, "decvice_inform",pbuf);
>>>>>>> dd00507a94c22f8e56aa46118ecc51c4e4d9c635
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
