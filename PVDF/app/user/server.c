/*
 * server.c
 *
 *  Created on: 2019��6��12��
 *      Author: pengfeizhao
 */
#include "server.h"
void ICACHE_FLASH_ATTR server_recvcb(void*arg,char*pdata,unsigned short len){
	os_printf("%s\r\n",pdata);

}
void ICACHE_FLASH_ATTR server_sentcb(void*arg){
	os_printf("���ͳɹ�");
}
void ICACHE_FLASH_ATTR server_disconcb(void*arg){
	struct	ip_info	ap_ip;
	os_printf("�Ͽ�����");

}

void  ICACHE_FLASH_ATTR server_listen(void *arg){
	espconn_regist_recvcb((struct espconn*)arg,server_recvcb);
	espconn_regist_sentcb((struct espconn*)arg,server_sentcb);
	espconn_regist_disconcb((struct espconn*)arg,server_disconcb);
}
void  ICACHE_FLASH_ATTR server_reconn(void *arg,sint8 err){
	os_printf("���Ӵ��󣬴������Ϊ%d\r\n",err);
}
void  ICACHE_FLASH_ATTR server_init(struct ip_addr *local_ip,int port)
{
	//espconn��������
	user_tcp_conn.type=ESPCONN_TCP;
	user_tcp_conn.state=ESPCONN_NONE;

	user_tcp_conn.proto.tcp=(esp_tcp *)os_zalloc(sizeof(esp_tcp));
     os_memcpy(user_tcp_conn.proto.tcp->local_ip,local_ip,4);
     user_tcp_conn.proto.tcp->local_port=port;
     //ע�����Ӻ������ӻص�����
     espconn_regist_connectcb(&user_tcp_conn,server_listen);
     espconn_regist_reconcb(&user_tcp_conn,server_reconn);
     //��������

     espconn_accept(&user_tcp_conn);
}



