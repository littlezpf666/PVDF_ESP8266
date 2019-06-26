/*
 * udp_config.c
 *
 *  Created on: 2019年6月16日
 *      Author: pengfeizhao
 */
#include "udp_config.h"
void ICACHE_FLASH_ATTR user_udp_send(void){
	struct	ip_info	ap_ip;
	char hwaddr[6];
	char DeviceBuffer[40]={0};
	char IPBuffer[10]={0};
	/*remot_info	*premot	=	NULL;
	wifi_get_macaddr(STATION_IF,hwaddr);
	os_sprintf(DeviceBuffer,"MAC:"MACSTR"!",MAC2STR(hwaddr));
	if (espconn_get_connection_info(&user_tcp_conn,&premot,0)==ESPCONN_OK)
	{
		os_strcat(DeviceBuffer,"busy\r\n");
		os_printf("TCP存在连接\r\n");
	}
	else
	{
		wifi_get_ip_info(STATION_IF,&ap_ip);
		os_sprintf(IPBuffer,"IP:"IPSTR"!\r\n",IP2STR(&ap_ip.ip));
		os_strcat(DeviceBuffer,"idle!");
		os_strcat(DeviceBuffer,IPBuffer);
		os_printf("TCP不存在连接\r\n");
	}*/
	espconn_sent(&user_udp_espconn,DeviceBuffer,strlen(DeviceBuffer));
}
void ICACHE_FLASH_ATTR user_ubp_sent_cb(void*arg){
	os_printf("UDP发送完毕!\r\n");
}
void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg,
		char * pdata,
		unsigned short len)
{
		struct	espconn	*pesp_conn	=	arg;
		remot_info	*premot	=	NULL;
		const char recev[7]="Mobile";
		if(!(os_strcmp(pdata,recev)))
		{
			os_printf("UDP已接受数据\"%s\"\r\n",pdata);
			if (espconn_get_connection_info(pesp_conn,&premot,0)==ESPCONN_OK){
				pesp_conn->proto.udp->remote_port  = premot->remote_port;
				pesp_conn->proto.udp->remote_ip[0] = premot->remote_ip[0];
				pesp_conn->proto.udp->remote_ip[1] = premot->remote_ip[1];
				pesp_conn->proto.udp->remote_ip[2] = premot->remote_ip[2];
				pesp_conn->proto.udp->remote_ip[3] = premot->remote_ip[3];
				espconn_sent(pesp_conn,"PVDF0",strlen("PVDF0"));
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
