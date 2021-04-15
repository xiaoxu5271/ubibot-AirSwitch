/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "Http.h"
#include "Smartconfig.h"
#include "Uart0.h"
#include "HLW8112.h"
#include "Led.h"
#include "E2prom.h"
#include "Switch.h"
#include "ota.h"
#include "user_app.h"
#include "Json_parse.h"
#include "user_key.h"
#include "My_Mqtt.h"
#include "Bluetooth.h"

void app_main()
{
	if (Check_First_Key())
	{
		ota_back();
	}

	Cache_muxtex = xSemaphoreCreateMutex();
	xMutex_Http_Send = xSemaphoreCreateMutex(); //创建HTTP发送互斥信号
	Net_sta_group = xEventGroupCreate();
	Send_Mqtt_Queue = xQueueCreate(1, sizeof(Mqtt_Msg));

	/* Print chip information */
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
		   chip_info.cores,
		   (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
		   (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

	printf("silicon revision %d, ", chip_info.revision);

	printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
		   (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	Led_Init();
	E2prom_Init();
	Read_Metadate_E2p();
	Read_Product_E2p();
	Read_Fields_E2p();
	Uart_Init();
	user_app_key_init();

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ble_app_init();
	init_wifi();

	initialise_http(); //须放在 采集任务建立之后
	initialise_mqtt();

	Switch_Init();
	HLW_Init();

	/* 判断是否有序列号和product id */
	if ((strlen(SerialNum) == 0) || (strlen(ProductId) == 0) || (strlen(WEB_SERVER) == 0)) //未获取到序列号或productid，未烧写序列号
	{
		while (1)
		{
			ESP_LOGE("Init", "no SerialNum or product id!");
			No_ser_flag = true;
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	}
}
