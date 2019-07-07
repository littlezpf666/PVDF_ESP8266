/*
 * server.c
 *
 *  Created on: 2019��6��12��
 *      Author: pengfeizhao
 */
#include "server.h"
#include "user_json.h"

struct bss_info*bss_link;
struct command rec_comm;
char wait_stm32=0;
/*****************************CJSON ����WiFi�б�***********************************/
int ICACHE_FLASH_ATTR
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
LOCAL struct jsontree_callback wifi_scan_callback =
JSONTREE_CALLBACK(wifi_scan_get, NULL);
JSONTREE_OBJECT(ssid_tree, JSONTREE_PAIR("ssid", &wifi_scan_callback),
		JSONTREE_PAIR("rssi", &wifi_scan_callback));

/*****************************CJSON 3����ָ�����ָ��wifi***********************************/
int ICACHE_FLASH_ATTR
wifi_conn_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {

	int type;
	int msgobj=0;
	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if (jsonparse_strcmp_value(parser, "MsgObj") == 0) {
				msgobj = 1;
			}
			if (msgobj) {
				//�������"MsgObj"����ͬһ�����ڱ���׽
				if (jsonparse_strcmp_value(parser, "ssid") == 0) {
					jsonparse_next(parser);
					jsonparse_next(parser);
					jsonparse_copy_value(parser, rec_comm.ssid,sizeof(rec_comm.ssid));
					os_printf("ssid:%s\r\n", rec_comm.ssid);

				} else if (jsonparse_strcmp_value(parser, "password") == 0) {
					jsonparse_next(parser);
					jsonparse_next(parser);
					jsonparse_copy_value(parser, rec_comm.password,
							sizeof(rec_comm.password));
					os_printf("password:%s\r\n", rec_comm.password);
					rec_comm.GET_SSID_INFO=1;

				}
			}

		}
	}
	return 0;
}

LOCAL struct jsontree_callback rece_3_tree_callback =
JSONTREE_CALLBACK(NULL, wifi_conn_parse);
JSONTREE_OBJECT(wifi_scan_tree, JSONTREE_PAIR("ssid", &rece_3_tree_callback),
		JSONTREE_PAIR("password", &rece_3_tree_callback)
		);
JSONTREE_OBJECT(rece_3_tree, JSONTREE_PAIR("MsgId", &rece_3_tree_callback),
		JSONTREE_PAIR("MsgObj", &wifi_scan_tree),
		JSONTREE_PAIR("MsStatus", &rece_3_tree_callback)
		);

/*****************************CJSON 1245����,34����ָ��***********************************/
int ICACHE_FLASH_ATTR
RXD_1245_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parser) {
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
int ICACHE_FLASH_ATTR
TXD_34_get(struct jsontree_context *js_ctx) {
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

	if (os_strncmp(path, "MsgId", 5) == 0) {
		jsontree_write_int(js_ctx, rec_comm.MSgId);
	}
	if (os_strncmp(path, "MsgObj", 6) == 0) {
		switch (rec_comm.MSgId)
		{
		  case 3:
			  jsontree_write_string(js_ctx, "success");
		  break;
		  case 4:
			  if(rec_comm.CON_STATUS==0)
				jsontree_write_string(js_ctx, "idle");
			  else
				jsontree_write_string(js_ctx, "busy");
		  break;
		}
	}

	if (os_strncmp(path, "MsgStatus", 9) == 0) {
		jsontree_write_string(js_ctx, "True");
		}

	return 0;
}
LOCAL struct jsontree_callback rece_1245_callback =
JSONTREE_CALLBACK(TXD_34_get, RXD_1245_parse);

JSONTREE_OBJECT(rece_1245_tree, JSONTREE_PAIR("MsgId", &rece_1245_callback),
		JSONTREE_PAIR("MsgObj", &rece_1245_callback),
		JSONTREE_PAIR("MsgStatus", &rece_1245_callback),
		);



/*****************************WIFI���ӳɹ��ص�****************************************/
void ICACHE_FLASH_ATTR wifi_connected(void *arg){
	struct station_info*st_info;
	struct ip_info	ap_ip;
	remot_info *premot;
	static uint8 count=0;
	uint8 status;
	char *pbuf = NULL;
	pbuf = (char *) os_zalloc(60);

	os_timer_disarm(&connect_timer);
	count++;

	status=wifi_station_get_connect_status();
	if(status==STATION_GOT_IP){//	STATION_GOT_IP
		if (espconn_get_connection_info(&user_tcp_conn,&premot,0)==ESPCONN_OK){
		os_printf("��ȡԶ��IP�ɹ�\r\n");
		os_printf("remote_port:%d,remote_IP:"IPSTR"\r\n", user_tcp_conn.proto.tcp->remote_port
			 ,IP2STR(user_tcp_conn.proto.tcp->remote_ip));
		user_tcp_conn.proto.tcp->remote_port  = premot->remote_port;
		user_tcp_conn.proto.tcp->remote_ip[0] = premot->remote_ip[0];
		user_tcp_conn.proto.tcp->remote_ip[1] = premot->remote_ip[1];
		user_tcp_conn.proto.tcp->remote_ip[2] = premot->remote_ip[2];
		user_tcp_conn.proto.tcp->remote_ip[3] = premot->remote_ip[3];
		os_printf("remote_port:%d,remote_IP:"IPSTR"\r\n", user_tcp_conn.proto.tcp->remote_port
					 ,IP2STR(user_tcp_conn.proto.tcp->remote_ip));

		//���ӳɹ���rec_comm.MsgObj�ó�success,��������JSON�ж�ʹ��
		//��Ϊһ���л�WiFiģʽԶ��IP�ͻ�ı䣬����Ҫ�ȵ����ͳɹ��ص��������ٸı�WIFIģʽ

		os_strncpy(rec_comm.MsgObj,"success",strlen("success"));
		json_ws_send((struct jsontree_value *) &rece_1245_tree, "success",pbuf);
		espconn_send(&user_tcp_conn,pbuf,strlen(pbuf));
		os_free(pbuf);
		}

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

/*****************************ɨ��ص�***********************************/
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status) {

	uint8 DeviceBuffer[2000]="";
	uint8 Buffer[2000]="";
	//remot_info�Ǳ��ض���������Ѿ����ǽṹ�ˣ������Լ�struct����
	remot_info *premot = NULL;
	char *pbuf = NULL;
	pbuf = (char *) os_zalloc(60);

	if (status == OK) {
		bss_link = (struct bss_info*) arg;
		//bss_link = bss_link->next.stqe_next;
		while (bss_link != NULL ) {
			json_ws_send((struct jsontree_value *) &ssid_tree, "ssid_list",pbuf);
			os_strcat(DeviceBuffer, pbuf);

			os_strcat(DeviceBuffer, ",\n");


			os_memset(pbuf, 0, 60);
			bss_link = bss_link->next.stqe_next;
		}
		*(DeviceBuffer+strlen(DeviceBuffer)-2)='\0';
		os_free(pbuf);
		if (espconn_get_connection_info(&user_tcp_conn,&premot,0)==ESPCONN_OK){

			user_tcp_conn.proto.tcp->remote_port  = premot->remote_port;
			user_tcp_conn.proto.tcp->remote_ip[0] = premot->remote_ip[0];
			user_tcp_conn.proto.tcp->remote_ip[1] = premot->remote_ip[1];
			user_tcp_conn.proto.tcp->remote_ip[2] = premot->remote_ip[2];
			user_tcp_conn.proto.tcp->remote_ip[3] = premot->remote_ip[3];
			os_printf("remote_port:%d,remote_IP:"IPSTR"\r\n", user_tcp_conn.proto.tcp->remote_port
										 ,IP2STR(user_tcp_conn.proto.tcp->remote_ip));
			}

		os_sprintf(Buffer,"{\"MsgId\":2,\n\"MsgObj\":[\n%s\n],\n\"MsgStatus\":\"true\"\n}",DeviceBuffer);

		/*һ�����������ǰһ�����ݷ��ͳɹ������� espconn_sent_callback ���ٵ��� espconn_send ������һ�����ݡ�*/
		espconn_send(&user_tcp_conn, Buffer, strlen(Buffer));
		os_free(DeviceBuffer);

	}
}

void ICACHE_FLASH_ATTR server_recvcb(void*arg, char*pdata, unsigned short len) {
		struct espconn *pesp_conn = arg;
		struct jsontree_context js;
		//������ap��Ϣ����
		struct station_config stationConf;
		//ɨ�����ýṹ
		struct scan_config config = {NULL,NULL,0,0};
		remot_info *premot = NULL;
		char pbuf_stm32[25] ;

		config.ssid="shnu-mobile";


		os_printf("remote_port:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->remote_port
						 ,IP2STR(pesp_conn->proto.tcp->remote_ip));

       //��ͨ��"MsgId"����Ҫ�����ֽṹ��

		rec_comm.MSgId=comtype_parse(pdata,"MsgId");
		switch(rec_comm.MSgId)
		{
		case 2:
			jsontree_setup(&js, (struct jsontree_value *) &rece_1245_tree, json_putchar);
			//������MsgObj
			json_parse(&js, pdata);
			if(!(os_strcmp(rec_comm.MsgObj,"scan")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					wifi_station_scan(&config, scan_done);
				}
			break;
		case 4:

			jsontree_setup(&js, (struct jsontree_value *) &rece_1245_tree, json_putchar);
			//������MsgObj
			json_parse(&js, pdata);
			if(!(os_strcmp(rec_comm.MsgObj,"close")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					espconn_disconnect(pesp_conn);
				}

			break;
		case 5:
			jsontree_setup(&js, (struct jsontree_value *) &rece_1245_tree, json_putchar);
			json_parse(&js, pdata);
			if(!(os_strcmp(rec_comm.MsgObj,"up")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					os_sprintf(pbuf_stm32,"zpf10000,%d,%d,%d \n",rec_comm.GET_SSID_INFO,TCP_PORT,UDP_PORT);
					wait_stm32=1;
				}
			if(!(os_strcmp(rec_comm.MsgObj,"down")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					os_sprintf(pbuf_stm32,"zpf01000,%d,%d,%d \n",rec_comm.GET_SSID_INFO,TCP_PORT,UDP_PORT);
					wait_stm32=1;
				}

			if(!(os_strcmp(rec_comm.MsgObj,"right")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					os_sprintf(pbuf_stm32,"zpf00100,%d,%d,%d \n",rec_comm.GET_SSID_INFO,TCP_PORT,UDP_PORT);
					wait_stm32=1;
				}
			if(!(os_strcmp(rec_comm.MsgObj,"left")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					os_sprintf(pbuf_stm32,"zpf00010,%d,%d,%d \n",rec_comm.GET_SSID_INFO,TCP_PORT,UDP_PORT);
					wait_stm32=1;
				}
			if(!(os_strcmp(rec_comm.MsgObj,"core")))
				{
				//��rec_comm.MsgObj��0����ֹrec_comm.MsgId��ȷ����rec_comm.MsgObj�޷���������ˢ����������
					memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
					os_sprintf(pbuf_stm32,"zpf00001,%d,%d,%d \n",rec_comm.GET_SSID_INFO,TCP_PORT,UDP_PORT);
					wait_stm32=1;
				}
			if(wait_stm32==1)
			os_printf("%s",pbuf_stm32);
			break;
		case 3: //����WIFI
			jsontree_setup(&js, (struct jsontree_value *) &rece_3_tree,
					json_putchar);
			json_parse(&js, pdata);
			if(rec_comm.GET_SSID_INFO){
				os_memcpy(&stationConf.ssid, rec_comm.ssid, 32);
				os_memcpy(&stationConf.password, rec_comm.password, 64);
				wifi_station_set_config_current(&stationConf);
				wifi_station_connect();
				os_timer_setfn(&connect_timer, wifi_connected, NULL);
				os_timer_arm(&connect_timer, 2000, NULL);
			}
			break;
		}
	//espconn_send(pesp_conn, pdata, os_strlen(pdata));
	/*os_printf("local_port3:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->local_port
	 ,IP2STR(pesp_conn->proto.tcp->remote_ip));*/
}
void ICACHE_FLASH_ATTR server_sentcb(void*arg) {
	struct espconn *pesp_conn = arg;
	os_printf("���ͳɹ�\r\n");
	if(!(os_strcmp(rec_comm.MsgObj,"success")))
	{
		wifi_set_opmode_current(STATION_MODE);

		memset(rec_comm.MsgObj,0,strlen(rec_comm.MsgObj));
		espconn_disconnect(pesp_conn);
	}

}
void ICACHE_FLASH_ATTR server_disconcb(void*arg) {
	struct ip_info ap_ip;
	rec_comm.CON_STATUS--;
	os_printf("�Ͽ�����\r\n");

}

void ICACHE_FLASH_ATTR server_listen(void *arg) {
	struct espconn *pesp_conn = arg;
    char *pbuf;
    pbuf=(char *)os_zalloc(60);
	os_printf("remote_port:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->remote_port
				 ,IP2STR(pesp_conn->proto.tcp->remote_ip));
	if (rec_comm.CON_STATUS == 0) {
		rec_comm.MSgId=4;
		json_ws_send((struct jsontree_value *) &rece_1245_tree, "idle",pbuf);
		espconn_send(&user_tcp_conn,pbuf,strlen(pbuf));
		rec_comm.CON_STATUS++;
		os_printf("�����豸״̬idle\r\n");
	}
	else {
		rec_comm.MSgId=4;
		json_ws_send((struct jsontree_value *) &rece_1245_tree, "busy",pbuf);
		espconn_send(pesp_conn,pbuf,strlen(pbuf));
		os_printf("�����豸״̬busy\r\n");
	}
	os_free(pbuf);
	espconn_regist_recvcb((struct espconn*) arg, server_recvcb);
	espconn_regist_sentcb((struct espconn*) arg, server_sentcb);
	espconn_regist_disconcb((struct espconn*) arg, server_disconcb);
}
void ICACHE_FLASH_ATTR server_reconn(void *arg, sint8 err) {
	os_printf("���Ӵ��󣬴������Ϊ%d\r\n", err);
}
void ICACHE_FLASH_ATTR server_init(int port) {

	//espconn��������
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	user_tcp_conn.proto.tcp->local_port = port;

	//ע�����Ӻ������ӻص�����
	espconn_regist_connectcb(&user_tcp_conn, server_listen);
	espconn_regist_reconcb(&user_tcp_conn, server_reconn);

	//��������
	espconn_accept(&user_tcp_conn);
	espconn_regist_time(&user_tcp_conn, 0, 0); //��ʱ�Ͽ�����ʱ�䣬�������Ҫ���÷����Ĭ��10s�����Զ��Ͽ���д0��������

}

