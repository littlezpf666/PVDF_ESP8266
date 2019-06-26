#include"driver/uart.h"
#include"user_main.h"
#define UDP_PORT 777
#define TCP_PORT 888
void ICACHE_FLASH_ATTR wifi_connected(void *arg){
	static uint8 count=0;
	//static uint8 machine_num=0;
	uint8 status;
	uint8 bssid[6];

	struct station_info*st_info;
	struct	ip_info	ap_ip;

	os_timer_disarm(&connect_timer);
	count++;
	//machine_num=wifi_softap_get_station_num();
	//os_printf("machine_num:%d\n",machine_num);
	status=wifi_station_get_connect_status();
	if(status==STATION_GOT_IP){//	STATION_GOT_IP
		/*st_info=wifi_softap_get_station_info();
		while(st_info!=NULL)
			{
			   os_printf("BSSID:\""MACSTR"\",%d\r\n",MAC2STR(st_info->bssid),st_info->ip.addr);
			   st_info=st_info->next.stqe_next;
			}*/
		wifi_get_ip_info(STATION_IF,&ap_ip);
		os_printf("IP:"IPSTR"\n\r",IP2STR(&ap_ip.ip));
		server_init(TCP_PORT);
		os_printf("TCP初始化成功\n\r");
		user_udp_init(UDP_PORT);
		os_printf("UDP初始化成功\n\r");
		//user_udp_send();
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

/***************************接收到station的连接每2s进入一次**********************************/
void ICACHE_FLASH_ATTR check_station(void *arg){
	struct	ip_info	ap_ip;
	struct station_info*st_info;
	os_timer_disarm(&station_check);
	//获取ESP8266连接的station的IP
	st_info=wifi_softap_get_station_info();
	if(st_info){
		//获取ESP8266本机IP
		wifi_get_ip_info(SOFTAP_IF,&ap_ip);

		//my_station_init(&(st_info->ip),&ap_ip.ip,8888);//注意第一个参数的赋值
		os_printf("IP:"IPSTR"",IP2STR(&ap_ip.ip));
		server_init(1213);

		os_free(st_info);
	}
	else{
		os_timer_arm(&station_check,2000,NULL);
	}

}
/*************************WiFi连接事件回调函数***********************************/
void ICACHE_FLASH_ATTR AP_be_connected_cb(System_Event_t *evt){

	switch(evt->event){
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("\r\nWIFI EVENT %d\r\n",evt->event);
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
	apConfig.max_connection =4;//最大连接数量，最大支持四个，默认四个
	apConfig.beacon_interval = 100;//信标间隔，默认为100
	apConfig.channel = 1;//信道，共支持1~13个信道
	apConfig.ssid_hidden = 0;//隐藏SSID，0：不隐藏  1：隐藏
	wifi_softap_set_config(&apConfig);

    system_init_done_cb(to_scan);
}

void user_rf_pre_init(){}
