#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "Smartconfig.h"
#include "Json_parse.h"
#include "HLW8112.h"
#include "esp_log.h"

#include "Led.h"

#define TAG "LED"

#if 1

#define GPIO_LED_R GPIO_NUM_12
#define GPIO_LED_B GPIO_NUM_2

#else

#define GPIO_LED_B 21
#define GPIO_LED_G 22
#define GPIO_LED_R 23

#endif

TaskHandle_t Led_Task_Handle = NULL;
uint8_t Last_Led_Status;

bool HLW_FLAG = false;
bool E2P_FLAG = false;
bool MECH_FLAG = false; //机械结构

bool Set_defaul_flag = false;
bool Net_sta_flag = false;
bool Cnof_net_flag = false;
bool No_ser_flag = false;

/*******************************************
硬件错误	红灯闪烁	1
恢复出厂设置	蓝灯闪烁	2
配置网络	红蓝交替闪烁	3
网络正常	蓝灯	4
网络异常	红灯	4

 
*******************************************/
static void Led_Task(void *arg)
{
    while (1)
    {
        //硬件错误
        if ((MECH_FLAG == false) || (E2P_FLAG == false) || (HLW_FLAG == false))
        {
            ESP_LOGI(TAG, "MECH_FLAG=:%d,E2P_FLAG=:%d,HLW_FLAG=:%d", MECH_FLAG, E2P_FLAG, HLW_FLAG);
            Led_Off();
            vTaskDelay(500 / portTICK_RATE_MS);
            Led_R_On();
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        //恢复出厂
        else if (Set_defaul_flag == true)
        {
            Led_Off();
            vTaskDelay(500 / portTICK_RATE_MS);
            Led_B_On();
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        //配网
        else if (Cnof_net_flag == true)
        {
            Led_Off();
            Led_B_On();
            vTaskDelay(500 / portTICK_RATE_MS);
            Led_Off();
            Led_R_On();
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        //开关以及网络
        else
        {
            // ESP_LOGI(TAG, "Net_sta_flag:%d", Net_sta_flag);
            if (Net_sta_flag == true)
            {
                Led_Off();
                Led_B_On();
                vTaskDelay(500 / portTICK_RATE_MS);
            }
            else
            {
                Led_Off();
                Led_R_On();
                vTaskDelay(500 / portTICK_RATE_MS);
            }
        }
    }
}

void Led_Init(void)
{
    gpio_config_t io_conf;

    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16
    io_conf.pin_bit_mask = (1ULL << GPIO_LED_B) | (1ULL << GPIO_LED_R);
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    xTaskCreate(Led_Task, "Led_Task", 3072, NULL, 2, &Led_Task_Handle);
}

void Led_R_On(void)
{
    gpio_set_level(GPIO_LED_R, 1);
}
void Led_R_Off(void)
{
    gpio_set_level(GPIO_LED_R, 0);
}

void Led_B_On(void)
{
    gpio_set_level(GPIO_LED_B, 1);
}

void Led_B_Off(void)
{
    gpio_set_level(GPIO_LED_B, 0);
}

void Led_Off(void)
{
    Led_B_Off();
    Led_R_Off();
}

void Turn_Off_LED(void)
{
    vTaskSuspend(Led_Task_Handle);
    Led_Off();
}

void Turn_ON_LED(void)
{
    vTaskResume(Led_Task_Handle);
}
