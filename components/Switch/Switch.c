#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
// #include "Cache_data.h"
// #include "ServerTimer.h"
#include "Switch.h"
// #include "Http.h"
// #include "Json_parse.h"
#include "Led.h"
// #include "Bluetooth.h"
// #include "E2prom.h"
// #include "Smartconfig.h"

// #include "Json_parse.h".

#define TAG "SWITCH"

static xQueueHandle gpio_evt_queue = NULL;
uint64_t SW_last_time = 0, SW_on_time = 0;
bool sw_sta = false; //1 合闸
bool f_sta = false;  //0 复位
bool m_sta = false;  //0 复位

void timer_trip_cb(void *arg);
esp_timer_handle_t timer_trip_handle = NULL; //定时器句柄
esp_timer_create_args_t timer_trip_arg = {
    .callback = &timer_trip_cb,
    .arg = NULL,
    .name = "Trip_Timer"};

void timer_motor_cb(void *arg);
esp_timer_handle_t timer_motor_handle = NULL; //定时器句柄
esp_timer_create_args_t timer_motor_arg = {
    .callback = &timer_motor_cb,
    .arg = NULL,
    .name = "Motor_Timer"};

void timer_test_cb(void *arg);
esp_timer_handle_t timer_test_handle = NULL; //定时器句柄
esp_timer_create_args_t timer_test_arg = {
    .callback = &timer_test_cb,
    .arg = NULL,
    .name = "Test_Timer"};

void timer_motor_cb(void *arg)
{
    //电机运转超时（电机损坏）
    gpio_set_level(M_IN1, 1);
    gpio_set_level(M_IN2, 1);
    ESP_LOGE(TAG, "MOTOR ERROR");
}

void timer_trip_cb(void *arg)
{
    //停止脱扣器供电
    gpio_set_level(EN_TRIP, 0);
}

void timer_test_cb(void *arg)
{
    //停止漏电测试
    gpio_set_level(EN_TEST, 0);
}

void Start_Leak_Test(void)
{
    gpio_set_level(EN_TEST, 1);
    esp_timer_start_once(timer_test_handle, TEST_EN_TIME);
}

void Switch_Relay(int8_t set_value)
{
    //切换
    if (set_value == -1)
    {
        if (sw_sta == 1) //分闸
        {
            gpio_set_level(EN_TRIP, 1);
            esp_timer_start_once(timer_trip_handle, TRIP_EN_TIME);
        }
        else //和闸
        {
            gpio_set_level(M_IN1, 0);
            gpio_set_level(M_IN2, 1);

            esp_timer_start_once(timer_motor_handle, MOTOR_OUT_TIME);
        }

        // memcpy(C_TYPE, "physical", 9);
    }

    //合闸
    else if (set_value >= 1 && set_value <= 100)
    {
        if (sw_sta == 1)
        {
            ESP_LOGI(TAG, "Switch is already ON");
        }
        else
        {
            gpio_set_level(M_IN1, 0);
            gpio_set_level(M_IN2, 1);

            esp_timer_start_once(timer_motor_handle, MOTOR_OUT_TIME);
        }
    }

    //分闸
    else if (set_value == 0)
    {
        if (sw_sta == 0)
        {
            ESP_LOGI(TAG, "Switch is already OFF");
        }
        else
        {
            gpio_set_level(EN_TRIP, 1);
            esp_timer_start_once(timer_trip_handle, TRIP_EN_TIME);
        }
    }

    //D触发器 上升沿

    // if (Binary_energy != NULL)
    // {
    //     xTaskNotifyGive(Binary_energy);
    // }
    // if (Binary_dp != NULL)
    // {
    //     xTaskNotifyGive(Binary_dp);
    // }
    // if (de_sw_s == 2)
    // {
    // E2P_WriteOneByte(LAST_SWITCH_ADD, mqtt_json_s.mqtt_switch_status); //写入开关状态
    // }

    // if (mqtt_json_s.mqtt_switch_status == 1)
    // {
    //     SW_last_time = (uint64_t)esp_timer_get_time();
    // }
    // else
    // {
    //     //累加单次开启时长
    //     SW_on_time += (uint64_t)esp_timer_get_time() - SW_last_time;
    // }
}

//读取，构建累积开启时长
// void Sw_on_quan_Task(void *pvParameters)
// {
//     char *filed_buff;
//     char *OutBuffer;
//     char *time_buff;
//     // uint8_t *SaveBuffer;
//     uint16_t len = 0;
//     cJSON *pJsonRoot;

//     while (1)
//     {
//         ulTaskNotifyTake(pdTRUE, -1);

//         //仍处于开启状态
//         if (mqtt_json_s.mqtt_switch_status == 1)
//         {
//             SW_on_time += (uint64_t)esp_timer_get_time() - SW_last_time;
//             SW_last_time = (uint64_t)esp_timer_get_time();
//         }
//         ESP_LOGI(TAG, "SW_on_time:%lld", SW_on_time);

//         if ((xEventGroupGetBits(Net_sta_group) & TIME_CAL_BIT) == TIME_CAL_BIT)
//         {
//             filed_buff = (char *)malloc(9);
//             time_buff = (char *)malloc(24);
//             Server_Timer_SEND(time_buff);
//             pJsonRoot = cJSON_CreateObject();
//             cJSON_AddStringToObject(pJsonRoot, "created_at", (const char *)time_buff);
//             snprintf(filed_buff, 9, "field%d", sw_on_f_num);
//             cJSON_AddItemToObject(pJsonRoot, filed_buff, cJSON_CreateNumber((uint64_t)((SW_on_time + 500000) / 1000000))); //四舍五入

//             OutBuffer = cJSON_PrintUnformatted(pJsonRoot); //cJSON_Print(Root)
//             if (OutBuffer != NULL)
//             {
//                 len = strlen(OutBuffer);
//                 ESP_LOGI(TAG, "len:%d\n%s\n", len, OutBuffer);
//                 xSemaphoreTake(Cache_muxtex, -1);
//                 DataSave((uint8_t *)OutBuffer, len);
//                 xSemaphoreGive(Cache_muxtex);
//                 cJSON_free(OutBuffer);
//             }
//             cJSON_Delete(pJsonRoot); //delete cjson root

//             free(filed_buff);
//             free(time_buff);
//             //清空统计
//             SW_on_time = 0;
//         }
//     }
// }
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
void HALL_Task(void *arg)
{
    uint32_t io_num;
    // printf("HALL_Task\r\n");
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));

            switch (io_num)
            {
            case HALL_M:
                m_sta = gpio_get_level(HALL_M);
                //齿轮复位
                if (m_sta == 0)
                {
                    esp_timer_stop(timer_motor_handle);
                    gpio_set_level(M_IN1, 1);
                    gpio_set_level(M_IN2, 1);
                    ESP_LOGI(TAG, "MOTOR IS REST");
                    // xEventGroupSetBits(Sw_sta_group, M_STA_BIT);
                }
                break;

            case HALL_S:
                sw_sta = gpio_get_level(HALL_S);
                break;

            case HALL_F:
                f_sta = gpio_get_level(HALL_F);
                break;

            default:
                break;
            }
        }
    }
}

void Switch_Init(void)
{
    Sw_sta_group = xEventGroupCreate();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    //电机
    io_conf.pin_bit_mask = ((1 << M_IN1) | (1 << M_IN2) | (1 << EN_TRIP) | (1 << EN_TEST));
    gpio_config(&io_conf);
    gpio_set_level(M_IN1, 0);
    gpio_set_level(M_IN2, 0);
    gpio_set_level(EN_TRIP, 0);
    gpio_set_level(EN_TEST, 0);

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.pin_bit_mask = ((1ULL << HALL_M) | (1ULL << HALL_S) | (1ULL << HALL_F));
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(HALL_Task, "HALL_Task", 2048, NULL, 10, NULL);
    esp_timer_create(&timer_trip_arg, &timer_trip_handle);
    esp_timer_create(&timer_motor_arg, &timer_motor_handle);
    esp_timer_create(&timer_test_arg, &timer_test_handle);

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(HALL_M, gpio_isr_handler, (void *)HALL_M);
    gpio_isr_handler_add(HALL_S, gpio_isr_handler, (void *)HALL_S);
    gpio_isr_handler_add(HALL_F, gpio_isr_handler, (void *)HALL_F);
    sw_sta = gpio_get_level(HALL_S);
    f_sta = gpio_get_level(HALL_F);
    m_sta = gpio_get_level(HALL_M);
    ESP_LOGI(TAG, "sw_sta:%d,f_sta:%d,m_sta:%d", sw_sta, f_sta, m_sta);
}