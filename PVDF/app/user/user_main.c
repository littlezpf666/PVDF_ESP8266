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
		server_init(&ap_ip.ip,TCP_PORT);
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
void ICACHE_FLASH_ATTR scan_done(void *arg,STATUS status ){
	uint8 ssid[33];
	struct	station_config	stationConf;
	if(status==OK)
	{
		struct bss_info*bss_link=(struct bss_info*)arg;
		while(bss_link!=NULL)
		{
			os_memset(ssid,0,33);
			if(os_strlen(bss_link->ssid)<=32)
			{
				os_memcpy(ssid,bss_link->ssid,os_strlen(bss_link));
			}
			else
			{
				os_memcpy(ssid,bss_link->ssid,32);
			}
			/*os_printf("+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
					bss_link->authmode,ssid,bss_link->rssi,
					MAC2STR(bss_link->bssid),bss_link->channel);*/
		bss_link=bss_link->next.stqe_next;
		}
		os_memcpy(&stationConf.ssid,"MiaoZi",32);
		os_memcpy(&stationConf.password,"12345678",64);
		wifi_station_set_config_current(&stationConf);
		wifi_station_connect();
		os_timer_setfn(&connect_timer,wifi_connected,NULL);
		os_timer_arm(&connect_timer,2000,NULL);
	}
}

void to_scan(void) {wifi_station_scan(NULL,scan_done);}

void user_init()
{
     struct softap_config config;
     struct softap_config apConfig;
    uart_init(115200,115200);
    os_printf("\r\n-------------------");
	os_printf("PVDF communication module");
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
