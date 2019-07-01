#include"driver/uart.h"
#include"user_main.h"


/***************************���յ�station������ÿ2s����һ��**********************************/
void ICACHE_FLASH_ATTR check_station(void *arg){
	struct	ip_info	ap_ip;
	struct station_info*st_info;
	os_timer_disarm(&station_check);
	//��ȡESP8266���ӵ�station��IP
	st_info=wifi_softap_get_station_info();
	if(st_info){
		//��ȡESP8266����IP
		/*wifi_get_ip_info(STATION_IF,&ap_ip);
				os_printf("IP:"IPSTR"\n\r",IP2STR(&ap_ip.ip));
		//my_station_init(&(st_info->ip),&ap_ip.ip,8888);//ע���һ�������ĸ�ֵ
		os_printf("IP:"IPSTR"",IP2STR(&ap_ip.ip));*/
		server_init(TCP_PORT);
		os_printf("TCP��ʼ���ɹ�\n\r");
		user_udp_init(UDP_PORT);
		os_printf("UDP��ʼ���ɹ�\n\r");
		os_free(st_info);
	}
	else{
		os_timer_arm(&station_check,2000,NULL);
	}

}
/*************************WiFi�����¼��ص�����***********************************/
void ICACHE_FLASH_ATTR AP_be_connected_cb(System_Event_t *evt){
	//static uint8 machine_num=0;
	switch(evt->event){
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("\r\nWIFI EVENT %d\r\n",evt->event);
		/*machine_num=wifi_softap_get_station_num();
		os_printf("machine_num:%d\n",machine_num);
		st_info=wifi_softap_get_station_info();
		 while(st_info!=NULL)
		 {
		 os_printf("BSSID:\""MACSTR"\",%d\r\n",MAC2STR(st_info->bssid),st_info->ip.addr);
		 st_info=st_info->next.stqe_next;
		 }*/
		os_timer_disarm(&station_check);
		os_timer_setfn(&station_check,check_station,NULL);
		os_timer_arm(&station_check,2000,NULL);
		break;
	default:
		break;
	}
}
void to_scan(void) {
	wifi_set_event_handler_cb(AP_be_connected_cb);

}

void user_init()
{
     struct softap_config apConfig;

    uart_init(115200,115200);
    os_printf("\r\n-------------------");
	os_printf("pvdf communication module");
	os_printf("-------------------\r\n");

	wifi_set_opmode_current(STATIONAP_MODE);

	os_strcpy(apConfig.ssid, "PVDF");
	apConfig.ssid_len = strlen("PVDF");
	os_strcpy(apConfig.password,"12345678");
	apConfig.authmode = AUTH_WPA_WPA2_PSK;
	apConfig.max_connection =4;//����������������֧���ĸ���Ĭ���ĸ�
	apConfig.beacon_interval = 100;//�ű�����Ĭ��Ϊ100
	apConfig.channel = 1;//�ŵ�����֧��1~13���ŵ�
	apConfig.ssid_hidden = 0;//����SSID��0��������  1������
	wifi_softap_set_config(&apConfig);

    system_init_done_cb(to_scan);
}

void user_rf_pre_init(){}
