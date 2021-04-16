#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <cJSON.h>
#include "esp_system.h"
#include "esp_log.h"

#include "ServerTimer.h"
#include "Http.h"

#include "esp_wifi.h"
#include "Smartconfig.h"
#include "E2prom.h"
#include "Http.h"
#include "Bluetooth.h"
#include "ota.h"
#include "Led.h"
// #include "tcp_bsp.h"
#include "my_base64.h"
#include "Switch.h"
#include "My_Mqtt.h"
#include "HLW8112.h"

#include "Json_parse.h"

#define TAG "Json_parse"

//metadata 相关变量
// uint8_t fn_set_flag = 0;     //metadata 读取标志，未读取则使用下面固定参数
uint32_t fn_dp = 120;     //数据发送频率
uint8_t fn_ang = 0;       //相位角测量，0关闭，1开启
uint8_t fn_freq = 0;      //交流电频率测量，0关闭，1开启
uint8_t fn_pf = 0;        //功率因数测量，0关闭，1开启
uint32_t fn_lc = 30;      //漏电保护设置 10-100 ，0关闭
uint32_t fn_oc = 0;       //过流保护设置，0关闭
uint32_t fn_ov = 0;       //过压保护设置，0关闭
uint32_t fn_sag = 60;     //欠压保护设置，0关闭
uint32_t fn_op = 0;       //过载保护设置，0关闭
uint32_t fn_sw_e = 0;     //电能信息采集周期：电压/电流/功率
uint32_t fn_sw_pc = 3600; //用电量统计
uint8_t cg_data_led = 1;  //发送数据 LED状态 0关闭，1打开
uint8_t de_sw_s = 0;      //开关默认上电状态
uint32_t fn_sw_on = 3600; //开启时长统计

// field num 相关参数
uint8_t f_sw_s = 1;     //开关状态 0关闭，1开启
uint8_t f_sw_v = 2;     //电压
uint8_t f_sw_c = 3;     //电流
uint8_t f_sw_p = 4;     //功率
uint8_t f_sw_lc = 5;    //漏电电流
uint8_t f_sw_pc = 6;    //累计用电量
uint8_t f_rssi_w = 7;   //WIFI 信号
uint8_t f_sw_on = 8;    //累计开启时长
uint8_t f_sw_ang = 9;   //相位角
uint8_t f_sw_freq = 11; //市电频率
uint8_t f_sw_pf = 10;   //功率因数

//
char SerialNum[SERISE_NUM_LEN] = {0};
char ProductId[PRODUCT_ID_LEN] = {0};
char ApiKey[API_KEY_LEN] = {0};
char ChannelId[CHANNEL_ID_LEN] = {0};
char USER_ID[USER_ID_LEN] = {0};
char WEB_SERVER[WEB_HOST_LEN] = {0};
char WEB_PORT[5] = {0};
char MQTT_SERVER[WEB_HOST_LEN] = {0};
char MQTT_PORT[5] = {0};
char BleName[100] = {0};
// char SIM_APN[32] = {0};
// char SIM_USER[32] = {0};
// char SIM_PWD[32] = {0};

//c-type
char C_TYPE[20] = "initial";
char WARN_CODE[25] = {0};
char ERROE_CODE[25] = {0};

//cali 相关 f1_a,f1_b,f1_a,f2_b,,,,,
f_cali f_cali_u[40] = {
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
    {1},
    {0},
};

const char f_cali_str[40][6] =
    {
        "f1_a", //
        "f1_b", //
        "f2_a",
        "f2_b",
        "f3_a",
        "f3_b",
        "f4_a",
        "f4_b",
        "f5_a",
        "f5_b",
        "f6_a",
        "f6_b",
        "f7_a",
        "f7_b",
        "f8_a",
        "f8_b",
        "f9_a",
        "f9_b",
        "f10_a",
        "f10_b",
        "f11_a",
        "f11_b",
        "f12_a",
        "f12_b",
        "f13_a",
        "f13_b",
        "f14_a",
        "f14_b",
        "f15_a",
        "f15_b",
        "f16_a",
        "f16_b",
        "f17_a",
        "f17_b",
        "f18_a",
        "f18_b",
        "f19_a",
        "f19_b",
        "f20_a",
        "f20_b"};

static short Parse_fields_num(char *ptrptr);
// static short Parse_cali_dat(char *ptrptr);

void Create_fields_num(char *read_buf);

static short Parse_metadata(char *ptrptr)
{
    if (NULL == ptrptr)
    {
        return 0;
    }

    cJSON *pJsonJson = cJSON_Parse(ptrptr);
    if (NULL == pJsonJson)
    {
        cJSON_Delete(pJsonJson); //delete pJson
        return 0;
    }
    cJSON *pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_dp"); //"fn_dp"
    if (NULL != pSubSubSub)
    {

        if ((unsigned long)pSubSubSub->valueint != fn_dp)
        {
            fn_dp = (unsigned long)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_DP_ADD, fn_dp, 4);
            ESP_LOGI(TAG, "fn_dp = %d\n", fn_dp);
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_ang"); //
    if (NULL != pSubSubSub)
    {

        if ((uint32_t)pSubSubSub->valueint != fn_ang)
        {
            fn_ang = (uint32_t)pSubSubSub->valueint;
            E2P_WriteOneByte(FN_ANG_ADD, fn_ang);
            ESP_LOGI(TAG, "fn_ang = %d\n", fn_ang);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_freq"); //""
    if (NULL != pSubSubSub)
    {

        if ((uint32_t)pSubSubSub->valueint != fn_freq)
        {
            fn_freq = (uint32_t)pSubSubSub->valueint;
            E2P_WriteOneByte(FN_FREQ_ADD, fn_freq);
            ESP_LOGI(TAG, "fn_freq = %d\n", fn_freq);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_pf"); //"fn_485_sth"
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_pf)
        {
            fn_pf = (uint32_t)pSubSubSub->valueint;
            E2P_WriteOneByte(FN_PF_ADD, fn_pf);
            ESP_LOGI(TAG, "fn_pf = %d\n", fn_pf);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_lc"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_lc)
        {
            fn_lc = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_LC_ADD, fn_lc, 4);
            HLW_Set_Flag = false;
            ESP_LOGI(TAG, "fn_lc = %d\n", fn_lc);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_oc"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_oc)
        {
            fn_oc = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_OC_ADD, fn_oc, 4);
            ESP_LOGI(TAG, "fn_oc = %d\n", fn_oc);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_ov"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_ov)
        {
            fn_ov = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_OV_ADD, fn_ov, 4);
            ESP_LOGI(TAG, "fn_ov = %d\n", fn_ov);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_sag"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_sag)
        {
            fn_sag = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_SAG_ADD, fn_sag, 4);
            ESP_LOGI(TAG, "fn_sag = %d\n", fn_sag);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_op"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_op)
        {
            fn_op = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_OP_ADD, fn_op, 4);
            ESP_LOGI(TAG, "fn_op = %d\n", fn_op);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_sw_e"); //
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_sw_e)
        {
            fn_sw_e = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_SW_E_ADD, fn_sw_e, 4);
            ESP_LOGI(TAG, "fn_sw_e = %d\n", fn_sw_e);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_sw_pc"); //"fn_sw_pc"
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_sw_pc)
        {
            fn_sw_pc = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_SW_PC_ADD, fn_sw_pc, 4);
            ESP_LOGI(TAG, "fn_sw_pc = %d\n", fn_sw_pc);
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "cg_data_led"); //"cg_data_led"
    if (NULL != pSubSubSub)
    {

        if ((uint8_t)pSubSubSub->valueint != cg_data_led)
        {
            cg_data_led = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(CG_DATA_LED_ADD, cg_data_led);
            ESP_LOGI(TAG, "cg_data_led = %d\n", cg_data_led);
            // if (cg_data_led == 0)
            // {
            //     Turn_Off_LED();
            // }
            // else
            // {
            //     Turn_ON_LED();
            // }
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "de_sw_s"); //"fn_sw_pc"
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != de_sw_s)
        {
            de_sw_s = (uint32_t)pSubSubSub->valueint;
            E2P_WriteOneByte(DE_SWITCH_STA_ADD, de_sw_s);
            ESP_LOGI(TAG, "de_sw_s = %d\n", de_sw_s);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "fn_sw_on"); //"fn_sw_pc"
    if (NULL != pSubSubSub)
    {
        if ((uint32_t)pSubSubSub->valueint != fn_sw_on)
        {
            fn_sw_on = (uint32_t)pSubSubSub->valueint;
            E2P_WriteLenByte(FN_SW_ON_ADD, fn_sw_on, 4);
            ESP_LOGI(TAG, "fn_sw_on = %d\n", fn_sw_on);
        }
    }

    cJSON_Delete(pJsonJson);
    // fn_set_flag = E2P_ReadOneByte(FN_SET_FLAG_ADD);
    // if (fn_set_flag != 1)
    // {
    //     fn_set_flag = 1;
    //     E2P_WriteOneByte(FN_SET_FLAG_ADD, fn_set_flag);
    // }

    return 1;
}

//解析蓝牙数据包
int32_t parse_objects_bluetooth(char *blu_json_data)
{
    cJSON *cjson_blu_data_parse = NULL;
    cJSON *cjson_blu_data_parse_command = NULL;
    ESP_LOGI(TAG, "start_ble_parse_json：\r\n%s\n", blu_json_data);

    char *resp_val = NULL;
    resp_val = strstr(blu_json_data, "{");
    if (resp_val == NULL)
    {
        ESP_LOGE("JSON", "DATA NO JSON");
        return 0;
    }
    cjson_blu_data_parse = cJSON_Parse(resp_val);

    if (cjson_blu_data_parse == NULL) //如果数据包不为JSON则退出
    {
        ESP_LOGE(TAG, "Json Formatting error2\n");
        cJSON_Delete(cjson_blu_data_parse);
        return BLU_JSON_FORMAT_ERROR;
    }
    else
    {
        cjson_blu_data_parse_command = cJSON_GetObjectItem(cjson_blu_data_parse, "command");
        ESP_LOGI(TAG, "command=%s\r\n", cjson_blu_data_parse_command->valuestring);
        char *Json_temp;
        Json_temp = cJSON_PrintUnformatted(cjson_blu_data_parse);
        if (Json_temp != NULL)
        {
            ParseTcpUartCmd(Json_temp);
            cJSON_free(Json_temp);
        }
    }
    cJSON_Delete(cjson_blu_data_parse);
    return 1;
}

//解析激活返回数据
esp_err_t parse_objects_http_active(char *http_json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    cJSON *json_data_parse_time_value = NULL;
    cJSON *json_data_parse_channel_channel_write_key = NULL;
    cJSON *json_data_parse_channel_channel_id_value = NULL;
    cJSON *json_data_parse_channel_metadata = NULL;
    cJSON *json_data_parse_channel_value = NULL;
    cJSON *json_data_parse_channel_user_id = NULL;
    //char *json_print;

    // ESP_LOGI(TAG,"start_parse_active_http_json\r\n");
    char *resp_val = NULL;
    resp_val = strstr(http_json_data, "{\"result\":\"success\",");
    if (resp_val == NULL)
    {
        ESP_LOGE("JSON", "DATA NO JSON");
        return ESP_FAIL;
    }
    json_data_parse = cJSON_Parse(resp_val);

    if (json_data_parse == NULL)
    {
        // ESP_LOGI(TAG,"Json Formatting error3\n");
        ESP_LOGE(TAG, "%s", http_json_data);
        cJSON_Delete(json_data_parse);
        return ESP_FAIL;
    }
    else
    {
        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "result");
        if (!(strcmp(json_data_parse_value->valuestring, "success")))
        {
            ESP_LOGI(TAG, "active:success\r\n");

            json_data_parse_time_value = cJSON_GetObjectItem(json_data_parse, "server_time");
            Server_Timer_GET(json_data_parse_time_value->valuestring);
        }
        else
        {
            ESP_LOGE(TAG, "active:error\r\n");
            cJSON_Delete(json_data_parse);
            return ESP_FAIL;
        }

        if (cJSON_GetObjectItem(json_data_parse, "channel") != NULL)
        {
            json_data_parse_channel_value = cJSON_GetObjectItem(json_data_parse, "channel");

            //ESP_LOGI(TAG,"%s\r\n", cJSON_Print(json_data_parse_channel_value));

            json_data_parse_channel_channel_write_key = cJSON_GetObjectItem(json_data_parse_channel_value, "write_key");
            json_data_parse_channel_channel_id_value = cJSON_GetObjectItem(json_data_parse_channel_value, "channel_id");
            json_data_parse_channel_metadata = cJSON_GetObjectItem(json_data_parse_channel_value, "metadata");
            json_data_parse_channel_user_id = cJSON_GetObjectItem(json_data_parse_channel_value, "user_id");

            Parse_metadata(json_data_parse_channel_metadata->valuestring);

            //写入API-KEY
            if (strcmp(ApiKey, json_data_parse_channel_channel_write_key->valuestring) != 0)
            {
                strcpy(ApiKey, json_data_parse_channel_channel_write_key->valuestring);
                ESP_LOGI(TAG, "api_key:%s\n", ApiKey);
                // E2P_Write(API_KEY_ADD, (uint8_t *)ApiKey, API_KEY_LEN);
            }

            //写入channelid
            if (strcmp(ChannelId, json_data_parse_channel_channel_id_value->valuestring) != 0)
            {
                strcpy(ChannelId, json_data_parse_channel_channel_id_value->valuestring);
                ESP_LOGI(TAG, "ChannelId:%s\n", ChannelId);
                // E2P_Write(CHANNEL_ID_ADD, (uint8_t *)ChannelId, CHANNEL_ID_LEN);
            }

            //写入user_id
            if (strcmp(USER_ID, json_data_parse_channel_user_id->valuestring) != 0)
            {
                strcpy(USER_ID, json_data_parse_channel_user_id->valuestring);
                ESP_LOGI(TAG, "USER_ID:%s\n", USER_ID);
                // E2P_Write(USER_ID_ADD, (uint8_t *)USER_ID, USER_ID_LEN);
            }
        }
    }
    cJSON_Delete(json_data_parse);
    return ESP_OK;
}

//解析http-post返回数据
esp_err_t parse_objects_http_respond(char *http_json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;
    // cJSON *json_data_parse_errorcode = NULL;

    char *resp_val = NULL;
    resp_val = strstr(http_json_data, "{\"result\":\"success\",\"server_time\":");
    if (resp_val == NULL)
    {
        ESP_LOGE(TAG, "%d", __LINE__);
        return ESP_FAIL;
    }
    json_data_parse = cJSON_Parse(resp_val);

    if (json_data_parse == NULL)
    {

        // ESP_LOGI(TAG,"Json Formatting error3\n");
        ESP_LOGE(TAG, "%s", http_json_data);
        cJSON_Delete(json_data_parse);
        return ESP_FAIL;
    }
    else
    {
        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "metadata");
        if (json_data_parse_value != NULL)
        {
            // ESP_LOGI(TAG,"metadata: %s\n", json_data_parse_value->valuestring);
            Parse_metadata(json_data_parse_value->valuestring);
        }

        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "sensors"); //sensors
        if (NULL != json_data_parse_value)
        {
            Parse_fields_num(json_data_parse_value->valuestring); //parse sensors
        }

        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "command"); //
        if (NULL != json_data_parse_value)
        {
            char *mqtt_json;
            mqtt_json = cJSON_PrintUnformatted(json_data_parse_value);
            if (mqtt_json != NULL)
            {
                ESP_LOGI(TAG, "%s", mqtt_json);
                parse_objects_mqtt(mqtt_json, false); //parse mqtt
                cJSON_free(mqtt_json);
            }
        }

        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "cali"); //cali
        if (NULL != json_data_parse_value)
        {
            ESP_LOGI(TAG, "cali: %s\n", json_data_parse_value->valuestring);
            // Parse_cali_dat(json_data_parse_value->valuestring); //parse cali
        }
    }

    cJSON_Delete(json_data_parse);
    return ESP_OK;
}

esp_err_t parse_objects_heart(char *json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_value = NULL;

    char *resp_val = NULL;
    resp_val = strstr(json_data, "{\"result\":\"success\",\"server_time\":");
    if (resp_val == NULL)
    {
        ESP_LOGE("JSON", "DATA NO JSON");
        return ESP_FAIL;
    }
    json_data_parse = cJSON_Parse(resp_val);

    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {

        ESP_LOGE(TAG, "Json Formatting error4\n");

        cJSON_Delete(json_data_parse);
        return ESP_FAIL;
    }
    else
    {
        json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "result");
        if (!(strcmp(json_data_parse_value->valuestring, "success")))
        {
            json_data_parse_value = cJSON_GetObjectItem(json_data_parse, "server_time");
            Server_Timer_GET(json_data_parse_value->valuestring);
        }
        else
        {
            ESP_LOGE(TAG, "active:error\r\n");
            cJSON_Delete(json_data_parse);
            return ESP_FAIL;
        }
    }
    cJSON_Delete(json_data_parse);
    return ESP_OK;
}

//解析MQTT指令
//sw_flag ：false,忽略开关执行指令
esp_err_t parse_objects_mqtt(char *mqtt_json_data, bool sw_flag)
{
    cJSON *json_data_parse = NULL;

    char *resp_val = NULL;
    resp_val = strstr(mqtt_json_data, "{\"command_id\":");
    if (resp_val == NULL)
    {
        // ESP_LOGE("JSON", "DATA NO JSON");
        return 0;
    }
    json_data_parse = cJSON_Parse(resp_val);

    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {
        ESP_LOGI(TAG, "Json Formatting error5\n");
        cJSON_Delete(json_data_parse);
        return 0;
    }

    cJSON *pSubSubSub = cJSON_GetObjectItem(json_data_parse, "command_id"); //
    if (pSubSubSub != NULL)
    {
        memset(mqtt_json_s.mqtt_command_id, 0, sizeof(mqtt_json_s.mqtt_command_id));
        strncpy(mqtt_json_s.mqtt_command_id, pSubSubSub->valuestring, strlen(pSubSubSub->valuestring));
        post_status = POST_NORMAL;
        // if (Binary_dp != NULL)
        // {
        //     xTaskNotifyGive(Binary_dp);
        // }
    }

    pSubSubSub = cJSON_GetObjectItem(json_data_parse, "command_string"); //
    if (pSubSubSub != NULL)
    {
        // ESP_LOGI(TAG, "command_string=%s", pSubSubSub->valuestring);
        ParseTcpUartCmd(pSubSubSub->valuestring);

        cJSON *json_data_parse_1 = cJSON_Parse(pSubSubSub->valuestring);
        if (json_data_parse_1 != NULL)
        {
            pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "action"); //
            if (pSubSubSub != NULL)
            {
                if (strcmp(pSubSubSub->valuestring, "ota") == 0)
                {
                    pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "url"); //
                    if (pSubSubSub != NULL)
                    {
                        strcpy(mqtt_json_s.mqtt_ota_url, pSubSubSub->valuestring);
                        ESP_LOGI(TAG, "OTA_URL=%s", mqtt_json_s.mqtt_ota_url);

                        // pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "size"); //
                        // if (pSubSubSub != NULL)
                        // {
                        // mqtt_json_s.mqtt_file_size = (uint32_t)pSubSubSub->valuedouble;
                        // ESP_LOGI(TAG, "OTA_SIZE=%d", mqtt_json_s.mqtt_file_size);

                        pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "version"); //
                        if (pSubSubSub != NULL)
                        {
                            if (strcmp(pSubSubSub->valuestring, FIRMWARE) != 0) //与当前 版本号 对比
                            {
                                ESP_LOGI(TAG, "OTA_VERSION=%s", pSubSubSub->valuestring);
                                if (Binary_dp != NULL)
                                {
                                    xTaskNotifyGive(Binary_dp);
                                }
                                ota_start(); //启动OTA
                            }
                        }
                        // }
                    }
                }
                else if (strcmp(pSubSubSub->valuestring, "command") == 0 && sw_flag == true)
                {
                    pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "c_type");
                    if (pSubSubSub != NULL)
                    {
                        snprintf(C_TYPE, sizeof(C_TYPE), "%s", pSubSubSub->valuestring);
                        // ESP_LOGI(TAG, "C_TYPE=%s", pSubSubSub->valuestring);
                    }
                    pSubSubSub = cJSON_GetObjectItem(json_data_parse_1, "set_state");
                    if (pSubSubSub != NULL)
                    {
                        Switch_Relay(pSubSubSub->valueint);
                    }
                }
            }
        }
        cJSON_Delete(json_data_parse_1);
    }
    cJSON_Delete(json_data_parse);

    return 1;
}

uint16_t Create_Status_Json(char *status_buff, uint16_t buff_len, bool filed_flag)
{
    uint8_t mac_sys[6] = {0};
    char *ssid64_buff;
    esp_read_mac(mac_sys, 0); //获取芯片内部默认出厂MAC
    ssid64_buff = (char *)malloc(64);
    memset(ssid64_buff, 0, 64);
    base64_encode(wifi_data.wifi_ssid, strlen(wifi_data.wifi_ssid), ssid64_buff, 64);

    snprintf(status_buff, buff_len, "],\"status\":\"mac=%02X:%02X:%02X:%02X:%02X:%02X,lock=%d,c_type=%s",
             mac_sys[0],
             mac_sys[1],
             mac_sys[2],
             mac_sys[3],
             mac_sys[4],
             mac_sys[5],
             f_sta,
             C_TYPE);

    if (strlen(WARN_CODE) != 0)
    {
        snprintf(status_buff + strlen(status_buff), buff_len - strlen(status_buff), ",warning=%s", WARN_CODE);
        memset(WARN_CODE, 0, sizeof(WARN_CODE));
    }
    if (strlen(ERROE_CODE) != 0)
    {
        snprintf(status_buff + strlen(status_buff), buff_len - strlen(status_buff), ",errors=%s", ERROE_CODE);
        memset(ERROE_CODE, 0, sizeof(ERROE_CODE));
    }

    snprintf(status_buff + strlen(status_buff), buff_len - strlen(status_buff), "\",\"ssid_base64\":\"%s\"", ssid64_buff);

    if (filed_flag == true)
    {
        char *field_buff;
        field_buff = (char *)malloc(FILED_BUFF_SIZE);
        memset(field_buff, 0, FILED_BUFF_SIZE);
        Create_fields_num(field_buff);

        snprintf(status_buff + strlen(status_buff), buff_len - strlen(status_buff), ",\"sensors\":[%s]}",
                 field_buff);

        free(field_buff);
    }
    else
    {
        snprintf(status_buff + strlen(status_buff), buff_len - strlen(status_buff), "}");
    }

    free(ssid64_buff);

    return strlen(status_buff);
}

void Create_NET_Json(void)
{
    if ((xEventGroupGetBits(Net_sta_group) & TIME_CAL_BIT) == TIME_CAL_BIT)
    {
        char *filed_buff;
        char *OutBuffer;
        char *time_buff;
        uint16_t len = 0;
        cJSON *pJsonRoot;
        wifi_ap_record_t wifidata_t;
        char *status_buf = (char *)malloc(50);
        filed_buff = (char *)malloc(9);
        time_buff = (char *)malloc(24);
        Server_Timer_SEND(time_buff);
        pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "created_at", (const char *)time_buff);

        if ((xEventGroupGetBits(Net_sta_group) & ACTIVED_BIT) == ACTIVED_BIT)
        {

            esp_wifi_sta_get_ap_info(&wifidata_t);
            snprintf(filed_buff, 9, "field%d", f_rssi_w);
            cJSON_AddItemToObject(pJsonRoot, filed_buff, cJSON_CreateNumber(wifidata_t.rssi));
        }
        cJSON_AddItemToObject(pJsonRoot, "field1", cJSON_CreateNumber(sw_sta));
        snprintf(status_buf, 50, "c_type=%s", C_TYPE);
        cJSON_AddItemToObject(pJsonRoot, "status", cJSON_CreateString(status_buf));

        OutBuffer = cJSON_PrintUnformatted(pJsonRoot); //cJSON_Print(Root)
        if (OutBuffer != NULL)
        {
            len = strlen(OutBuffer);
            ESP_LOGI(TAG, "len:%d\n%s\n", len, OutBuffer);

            if (xSemaphoreTake(Cache_muxtex, 10 / portTICK_RATE_MS) == pdTRUE)
            {
                DataSave((uint8_t *)OutBuffer, len);
                xSemaphoreGive(Cache_muxtex);
            }
            else
            {
                ESP_LOGE(TAG, "%d,Cache_muxtex", __LINE__);
            }

            cJSON_free(OutBuffer);
        }
        cJSON_Delete(pJsonRoot); //delete cjson root

        free(status_buf);
        free(filed_buff);
        free(time_buff);
    }
}

void Create_Switch_Json(void)
{
    if ((xEventGroupGetBits(Net_sta_group) & TIME_CAL_BIT) == TIME_CAL_BIT)
    {
        char *OutBuffer;
        uint16_t len = 0;
        cJSON *pJsonRoot;
        char *status_buf = (char *)malloc(50);
        char *time_buff = (char *)malloc(24);
        Server_Timer_SEND(time_buff);

        pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "created_at", (const char *)time_buff);

        cJSON_AddItemToObject(pJsonRoot, "field1", cJSON_CreateNumber(sw_sta));
        snprintf(status_buf, 50, "c_type=%s", C_TYPE);
        cJSON_AddItemToObject(pJsonRoot, "status", cJSON_CreateString(status_buf));

        OutBuffer = cJSON_PrintUnformatted(pJsonRoot); //cJSON_Print(Root)
        if (OutBuffer != NULL)
        {
            len = strlen(OutBuffer);
            ESP_LOGI(TAG, "len:%d\n%s\n", len, OutBuffer);
            if (xSemaphoreTake(Cache_muxtex, 10 / portTICK_RATE_MS) == pdTRUE)
            {
                DataSave((uint8_t *)OutBuffer, len);
                xSemaphoreGive(Cache_muxtex);
            }
            else
            {
                ESP_LOGE(TAG, "%d,Cache_muxtex", __LINE__);
            }
            cJSON_free(OutBuffer);
        }
        cJSON_Delete(pJsonRoot); //delete cjson root

        free(status_buf);
        free(time_buff);
    }
}

/*******************************************************************************
// create sensors fields num 
*******************************************************************************/
void Create_fields_num(char *read_buf)
{
    char *out_buf;
    uint16_t data_len;
    cJSON *pJsonRoot;

    pJsonRoot = cJSON_CreateObject();

    cJSON_AddNumberToObject(pJsonRoot, "sw_s", f_sw_s);       //sw_s_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_v", f_sw_v);       //sw_v_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_c", f_sw_c);       //sw_c_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_p", f_sw_p);       //sw_p_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_lc", f_sw_lc);     //sw_pc_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_pc", f_sw_pc);     //rssi_w_f_num
    cJSON_AddNumberToObject(pJsonRoot, "rssi_w", f_rssi_w);   //rssi_g_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_on", f_sw_on);     //r1_light_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_ang", f_sw_ang);   //r1_th_t_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_freq", f_sw_freq); //r1_th_h_f_num
    cJSON_AddNumberToObject(pJsonRoot, "sw_pf", f_sw_pf);     //r1_sth_t_f_num

    out_buf = cJSON_PrintUnformatted(pJsonRoot); //cJSON_Print(Root)
    if (out_buf != NULL)
    {
        data_len = strlen(out_buf); //error
        memcpy(read_buf, out_buf, data_len);
        cJSON_free(out_buf);
    }
    cJSON_Delete(pJsonRoot); //delete cjson root
}

/*******************************************************************************
// create cali num 
*******************************************************************************/
void Create_cali_buf(char *read_buf)
{
    char *out_buf;
    cJSON *pJsonRoot;

    pJsonRoot = cJSON_CreateObject();

    for (uint8_t i = 0; i < 40; i++)
    {
        cJSON_AddNumberToObject(pJsonRoot, f_cali_str[i], f_cali_u[i].val); //sw_s_f_num
    }

    out_buf = cJSON_PrintUnformatted(pJsonRoot); //cJSON_Print(Root)
    if (out_buf != NULL)
    {
        strcpy(read_buf, out_buf); //error
        cJSON_free(out_buf);
    }
    cJSON_Delete(pJsonRoot); //delete cjson root
}

esp_err_t ParseTcpUartCmd(char *pcCmdBuffer, ...)
{
    char *Ret_OK = "{\"status\":0,\"code\": 0}\r\n";
    char *Ret_Fail = "{\"status\":1,\"code\":1}\r\n";

    va_list argp;
    /*argp指向传入的第一个可选参数，msg是最后一个确定的参数*/
    va_start(argp, pcCmdBuffer);
    int sock = 0;
    int tcp_flag = va_arg(argp, int);
    if (tcp_flag == 1)
    {
        sock = va_arg(argp, int);
        ESP_LOGI(TAG, "sock: %d\n", sock);
    }
    va_end(argp);

    esp_err_t ret = ESP_FAIL;
    if (NULL == pcCmdBuffer) //null
    {
        ESP_LOGE(TAG, "%d", __LINE__);
        ret = ESP_FAIL;
        return ret;
    }

    cJSON *pJson = cJSON_Parse(pcCmdBuffer); //parse json data
    if (NULL == pJson)
    {
        ESP_LOGE(TAG, "%d", __LINE__);
        cJSON_Delete(pJson); //delete pJson
        ret = ESP_FAIL;
        return ret;
    }

    cJSON *pSub = cJSON_GetObjectItem(pJson, "Command"); //"Command"
    if (NULL != pSub)
    {
        if (!strcmp((char const *)pSub->valuestring, "SetupProduct")) //Command:SetupProduct
        {
            pSub = cJSON_GetObjectItem(pJson, "Password"); //"Password"
            if (NULL != pSub)
            {
                if (!strcmp((char const *)pSub->valuestring, "CloudForce"))
                {
                    // E2prom_set_defaul(false);
                    pSub = cJSON_GetObjectItem(pJson, "ProductID"); //"ProductID"
                    if (NULL != pSub)
                    {
                        ESP_LOGI(TAG, "ProductID= %s, len= %d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(PRODUCT_ID_ADDR, (uint8_t *)pSub->valuestring, PRODUCT_ID_LEN); //save Prfield1uctID
                    }

                    pSub = cJSON_GetObjectItem(pJson, "SeriesNumber"); //"SeriesNumber"
                    if (NULL != pSub)
                    {
                        ESP_LOGI(TAG, "SeriesNumber= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(SERISE_NUM_ADDR, (uint8_t *)pSub->valuestring, SERISE_NUM_LEN); //save
                    }

                    pSub = cJSON_GetObjectItem(pJson, "Host"); //"Host"
                    if (NULL != pSub)
                    {
                        ESP_LOGI(TAG, "Host= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(WEB_HOST_ADD, (uint8_t *)pSub->valuestring, WEB_HOST_LEN); //save
                    }

                    pSub = cJSON_GetObjectItem(pJson, "Port"); //"WEB PORT"
                    if (NULL != pSub)
                    {

                        ESP_LOGI(TAG, "Port= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(WEB_PORT_ADD, (uint8_t *)pSub->valuestring, 5); //save
                    }

                    pSub = cJSON_GetObjectItem(pJson, "MqttHost"); //"mqtt Host"
                    if (NULL != pSub)
                    {
                        ESP_LOGI(TAG, "MqttHost= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(MQTT_HOST_ADD, (uint8_t *)pSub->valuestring, WEB_HOST_LEN); //save
                    }

                    pSub = cJSON_GetObjectItem(pJson, "MqttPort"); //"Host"
                    if (NULL != pSub)
                    {
                        ESP_LOGI(TAG, "MqttPort= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                        E2P_Write(MQTT_PORT_ADD, (uint8_t *)pSub->valuestring, 5); //save
                    }
                    if (tcp_flag)
                    {
                        Tcp_Send(sock, Ret_OK);
                    }
                    printf("%s", Ret_OK);

                    vTaskDelay(3000 / portTICK_RATE_MS);
                    cJSON_Delete(pJson);
                    esp_restart(); //芯片复位 函数位于esp_system.h

                    ret = ESP_OK;
                }
                else
                {
                    ret = ESP_FAIL;
                    //密码错误
                    if (tcp_flag)
                    {
                        Tcp_Send(sock, Ret_Fail);
                    }
                    printf("%s", Ret_Fail);
                }
            }
        }

        else if (!strcmp((char const *)pSub->valuestring, "SetupWifi")) //Command:SetupWifi
        {
            pSub = cJSON_GetObjectItem(pJson, "SSID"); //"SSID"
            if (NULL != pSub)
            {
                bzero(wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
                strcpy(wifi_data.wifi_ssid, pSub->valuestring);
                E2P_Write(WIFI_SSID_ADD, (uint8_t *)wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
                ESP_LOGI(TAG, "WIFI_SSID = %s\r\n", pSub->valuestring);
            }

            pSub = cJSON_GetObjectItem(pJson, "password"); //"password"
            if (NULL != pSub)
            {
                bzero(wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
                strcpy(wifi_data.wifi_pwd, pSub->valuestring);
                E2P_Write(WIFI_PASSWORD_ADD, (uint8_t *)wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
                ESP_LOGI(TAG, "WIFI_PWD = %s\r\n", pSub->valuestring);
            }
            if (tcp_flag)
            {
                Tcp_Send(sock, Ret_OK);
                //AP配置，先关闭蓝牙
                ble_app_stop();
            }
            printf("%s", Ret_OK);
            vTaskDelay(1000 / portTICK_RATE_MS);
            Net_Switch();

            //重置网络

            ret = ESP_OK;
        }
        //{"command":"SetupHost","Host":"api.ubibot.io","Port":"80","MqttHost":"mqtt.ubibot.io","MqttPort":"1883"}
        else if (!strcmp((char const *)pSub->valuestring, "SetupHost")) //Command:SetupWifi
        {
            pSub = cJSON_GetObjectItem(pJson, "Host"); //"Host"
            if (NULL != pSub)
            {
                ESP_LOGI(TAG, "Host= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                E2P_Write(WEB_HOST_ADD, (uint8_t *)pSub->valuestring, WEB_HOST_LEN); //save SeriesNumber
            }

            pSub = cJSON_GetObjectItem(pJson, "Port"); //"WEB PORT"
            if (NULL != pSub)
            {
                ESP_LOGI(TAG, "Port= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                E2P_Write(WEB_PORT_ADD, (uint8_t *)pSub->valuestring, 5); //save
            }

            pSub = cJSON_GetObjectItem(pJson, "MqttHost"); //"mqtt Host"
            if (NULL != pSub)
            {
                ESP_LOGI(TAG, "MqttHost= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                E2P_Write(MQTT_HOST_ADD, (uint8_t *)pSub->valuestring, WEB_HOST_LEN); //save
            }

            pSub = cJSON_GetObjectItem(pJson, "MqttPort"); //"Host"
            if (NULL != pSub)
            {
                ESP_LOGI(TAG, "MqttPort= %s, len=%d\n", pSub->valuestring, strlen(pSub->valuestring));
                E2P_Write(MQTT_PORT_ADD, (uint8_t *)pSub->valuestring, 5); //save
            }

            if (tcp_flag)
            {
                Tcp_Send(sock, Ret_OK);
            }
            printf("%s", Ret_OK);

            vTaskDelay(3000 / portTICK_RATE_MS);
            cJSON_Delete(pJson);
            esp_restart(); //芯片复位 函数位于esp_system.h

            ret = ESP_OK;
        }

        //{"command":"ReadProduct"}
        else if (!strcmp((char const *)pSub->valuestring, "ReadProduct")) //Command:ReadProduct
        {
            char mac_buff[18];
            char *json_temp;
            uint8_t mac_sys[6] = {0};
            cJSON *root = cJSON_CreateObject();

            // E2P_Read(SERISE_NUM_ADDR, (uint8_t *)SerialNum, SERISE_NUM_LEN);
            // E2P_Read(PRODUCT_ID_ADDR, (uint8_t *)ProductId, PRODUCT_ID_LEN);
            // E2P_Read(WEB_HOST_ADD, (uint8_t *)WEB_SERVER, WEB_HOST_LEN);
            // E2P_Read(CHANNEL_ID_ADD, (uint8_t *)ChannelId, CHANNEL_ID_LEN);
            // E2P_Read(USER_ID_ADD, (uint8_t *)USER_ID, USER_ID_LEN);
            // E2P_Read(WEB_PORT_ADD, (uint8_t *)WEB_PORT, 5);
            // E2P_Read(MQTT_HOST_ADD, (uint8_t *)MQTT_SERVER, USER_ID_LEN);
            // E2P_Read(MQTT_PORT_ADD, (uint8_t *)MQTT_PORT, 5);

            esp_read_mac(mac_sys, 0); //获取芯片内部默认出厂MAC，
            snprintf(mac_buff,
                     sizeof(mac_buff),
                     "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac_sys[0],
                     mac_sys[1],
                     mac_sys[2],
                     mac_sys[3],
                     mac_sys[4],
                     mac_sys[5]);

            cJSON_AddItemToObject(root, "ProductID", cJSON_CreateString(ProductId));
            cJSON_AddItemToObject(root, "SeriesNumber", cJSON_CreateString(SerialNum));
            cJSON_AddItemToObject(root, "Host", cJSON_CreateString(WEB_SERVER));
            cJSON_AddItemToObject(root, "Port", cJSON_CreateString(WEB_PORT));
            cJSON_AddItemToObject(root, "MqttHost", cJSON_CreateString(MQTT_SERVER));
            cJSON_AddItemToObject(root, "MqttPort", cJSON_CreateString(MQTT_PORT));
            cJSON_AddItemToObject(root, "CHANNEL_ID", cJSON_CreateString(ChannelId));
            cJSON_AddItemToObject(root, "USER_ID", cJSON_CreateString(USER_ID));
            cJSON_AddItemToObject(root, "MAC", cJSON_CreateString(mac_buff));
            cJSON_AddItemToObject(root, "firmware", cJSON_CreateString(FIRMWARE));

            json_temp = cJSON_PrintUnformatted(root);
            if (json_temp != NULL)
            {
                if (tcp_flag)
                {
                    Tcp_Send(sock, json_temp);
                }
                printf("%s\r\n", json_temp);
                cJSON_free(json_temp);
            }
            cJSON_Delete(root); //delete pJson
            ret = ESP_OK;
        }
        //{"command":"CheckSensors"}
        else if (!strcmp((char const *)pSub->valuestring, "CheckSensors"))
        {
            cJSON *root = cJSON_CreateObject();
            char *json_temp;
            char wifi_ssid[33] = {0};
            int8_t wifi_rssi;

            bool result_flag = true;
            bool WIFI_flag = false;

            // xTaskNotifyGive(Binary_485_th);
            // xTaskNotifyGive(Binary_ext);
            // xTaskNotifyGive(Binary_energy);

            if (Check_Wifi((uint8_t *)wifi_ssid, &wifi_rssi))
            {
                WIFI_flag = true;
            }
            else
            {
                result_flag = false;
                WIFI_flag = false;
            }

            //构建返回结果
            if (result_flag == true)
            {
                cJSON_AddStringToObject(root, "result", "OK");
            }
            else
            {
                cJSON_AddStringToObject(root, "result", "ERROR");
            }

            if (WIFI_flag == true)
            {
                cJSON_AddStringToObject(root, "ssid", wifi_ssid);
                cJSON_AddNumberToObject(root, "rssi", wifi_rssi);
            }
            else
            {
                cJSON_AddStringToObject(root, "ssid", "NULL");
                cJSON_AddNumberToObject(root, "rssi", 0);
            }

            // if (ENERGY_flag == true)
            // {
            //     cJSON_AddStringToObject(root, "ENERGY", "OK");
            // }
            // else
            // {
            //     cJSON_AddStringToObject(root, "ENERGY", "ERROR");
            // }

            if (E2P_FLAG == true)
            {
                cJSON_AddStringToObject(root, "e2prom", "OK");
            }
            else
            {
                cJSON_AddStringToObject(root, "e2prom", "ERROR");
            }

            json_temp = cJSON_PrintUnformatted(root);
            if (json_temp != NULL)
            {
                if (tcp_flag)
                {
                    Tcp_Send(sock, json_temp);
                }
                printf("%s\r\n", json_temp);
                cJSON_free(json_temp);
            }
            cJSON_Delete(root); //delete pJson
            ret = ESP_OK;
        }

        else if (!strcmp((char const *)pSub->valuestring, "ScanWifiList"))
        {
            Scan_Wifi();
            ret = ESP_OK;
        }
        //{"command":"reboot"}
        else if (!strcmp((char const *)pSub->valuestring, "reboot"))
        {
            esp_restart();
            ret = ESP_OK;
        }
        //{"command":"AllReset"}
        else if (!strcmp((char const *)pSub->valuestring, "AllReset"))
        {
            E2prom_set_defaul(false);
            ret = ESP_OK;
        }
    }

    cJSON_Delete(pJson); //delete pJson

    return ret;
}

/*******************************************************************************
// parse sensors fields num
*******************************************************************************/
static short Parse_fields_num(char *ptrptr)
{
    if (NULL == ptrptr)
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Parse_fields_num:%s", ptrptr);
    cJSON *pJsonJson = cJSON_Parse(ptrptr);
    if (NULL == pJsonJson)
    {
        cJSON_Delete(pJsonJson); //delete pJson
        return ESP_FAIL;
    }

    cJSON *pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_s"); //"sw_s"
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_s)
        {
            f_sw_s = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_S_ADD, f_sw_s);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_v"); //"sw_v"
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_v)
        {
            f_sw_v = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_V_ADD, f_sw_v); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_c"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_c)
        {
            f_sw_c = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_C_ADD, f_sw_c); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_p"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_p)
        {
            f_sw_p = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_P_ADD, f_sw_p); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_lc"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_lc)
        {
            f_sw_lc = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_LC_ADD, f_sw_lc); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_pc"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_pc)
        {
            f_sw_pc = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_PC_ADD, f_sw_pc); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "rssi_w"); //"rssi_w"
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_rssi_w)
        {
            f_rssi_w = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_RSSI_W_ADD, f_rssi_w);
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_on"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_on)
        {
            f_sw_on = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_ON_ADD, f_sw_on); //
        }
    }

    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_ang"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_ang)
        {
            f_sw_ang = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_ANG_ADD, f_sw_ang); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "sw_freq"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_freq)
        {
            f_sw_freq = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_S_FREQ_ADD, f_sw_freq); //
        }
    }
    pSubSubSub = cJSON_GetObjectItem(pJsonJson, "f_sw_pf"); //""
    if (NULL != pSubSubSub)
    {
        if ((uint8_t)pSubSubSub->valueint != f_sw_pf)
        {
            f_sw_pf = (uint8_t)pSubSubSub->valueint;
            E2P_WriteOneByte(F_SW_PF_ADD, f_sw_pf); //
        }
    }

    cJSON_Delete(pJsonJson);

    return ESP_OK;
}

/*******************************************************************************
// parse sensors fields num
*******************************************************************************/
// static short Parse_cali_dat(char *ptrptr)
// {
//     cJSON *pSubSubSub;

//     if (NULL == ptrptr)
//     {
//         return ESP_FAIL;
//     }
//     cJSON *pJsonJson = cJSON_Parse(ptrptr);
//     if (NULL == pJsonJson)
//     {
//         cJSON_Delete(pJsonJson); //delete pJson
//         return ESP_FAIL;
//     }

//     for (uint8_t i = 0; i < 20; i++)
//     {
//         pSubSubSub = cJSON_GetObjectItem(pJsonJson, f_cali_str[i]); //
//         if (NULL != pSubSubSub)
//         {
//             if ((float)pSubSubSub->valuedouble != f_cali_u[i].val)
//             {
//                 f_cali_u[i].val = (float)pSubSubSub->valuedouble;
//                 E2P_WriteLenByte(5 * i + F1_A_ADDR, f_cali_u[i].dat, 4);
//             }
//         }
//     }

//     cJSON_Delete(pJsonJson);
//     return ESP_OK;
// }

//计算 filed 校准值
double Cali_filed(uint8_t filed_num, double filed_val)
{
    double filed_val_c;
    filed_val_c = (double)f_cali_u[(filed_num - 1) * 2].val * filed_val + f_cali_u[(filed_num - 1) * 2 + 1].val; //a*X+b

    return filed_val_c;
}

// get mid str 把src中 s1 到 s2之间的数据拷贝（包括s1不包括S2）到 sub中 ,返回 s2地址
char *mid(char *src, char *s1, char *s2, char *sub)
{
    char *sub1;
    char *sub2;
    uint16_t n;

    sub1 = strstr(src, s1);
    if (sub1 == NULL)
        return NULL;
    sub1 += strlen(s1);
    sub2 = strstr(sub1, s2);
    if (sub2 == NULL)
        return NULL;
    n = sub2 - sub1;
    strncpy(sub, sub1, n);
    sub[n] = 0;
    return sub2;
}

// return: NULL Not Found
//反向查找字符串 _MaxLen: 内存大小，_ReadLen:查找的长度
char *s_rstrstr(const char *_pBegin, int _MaxLen, int _ReadLen, const char *_szKey)
{
    if (NULL == _pBegin || NULL == _szKey || _MaxLen <= 0)
    {
        return NULL;
    }
    int i = _MaxLen - 1;
    int j = strlen(_szKey) - 1;
    int k = 0;
    // int s32CmpLen = strlen(_szKey);

    for (i = _MaxLen - 1; i >= (_MaxLen - _ReadLen); i--)
    {
        if (_pBegin[i] == _szKey[j])
        {
            for (j = strlen(_szKey) - 1, k = i; j >= 0; j--, k--)
            {
                if (_pBegin[k] != _szKey[j])
                {
                    j = strlen(_szKey) - 1;
                    break;
                }
                if (j == 0)
                {
                    return (char *)_pBegin + i - strlen(_szKey) + 1;
                }
            }
        }
    }

    return NULL;
}

// return: NULL Not Found
//反向逐字节查找字符串 _MaxLen: 内存大小，_ReadLen:查找的长度
char *s_rstrstr_2(const char *_pBegin, int _MaxLen, const char *_szKey)
{
    if (NULL == _pBegin || NULL == _szKey || _MaxLen <= 0)
    {
        return NULL;
    }
    int i;
    int j = strlen(_szKey) - 1;
    int k = 0;
    int s32CmpLen = strlen(_szKey);
    char *temp = 0;

    for (i = 0; i <= _MaxLen; i++)
    {
        temp = _pBegin - i;

        if (*temp == _szKey[j])
        {
            for (j = strlen(_szKey) - 1, k = i; j >= 0; j--, k++)
            {
                temp = _pBegin - k;
                if (*temp != _szKey[j])
                {
                    j = strlen(_szKey) - 1;
                    break;
                }
                if (j == 0)
                {
                    return temp;
                }
            }
        }
    }
    return NULL;
}

// return: NULL Not Found
//buff中正向查找字符串 _ReadLen:查找的长度
//返回 要查找的字符串之后的地址
char *s_strstr(const char *_pBegin, int _ReadLen, int *first_len, const char *_szKey)
{
    if (NULL == _pBegin || NULL == _szKey || _ReadLen <= 0)
    {
        return NULL;
    }
    int i = 0, j = 0, k = 0;
    int s32CmpLen = strlen(_szKey);

    for (i = 0; i < _ReadLen; i++)
    {
        if (_pBegin[i] == _szKey[j])
        {
            for (j = 0, k = i; j < s32CmpLen; j++, k++)
            {
                if (_pBegin[k] != _szKey[j])
                {
                    j = 0;
                    break;
                }
                if (j == s32CmpLen - 1)
                {
                    if (first_len != NULL)
                    {
                        // ESP_LOGI(TAG,"s_strstr,i=%d\n", i);
                        *first_len = i;
                    }
                    return (char *)_pBegin + i + s32CmpLen;
                }
            }
        }
    }

    return NULL;
}

//读取EEPROM中的metadata
void Read_Metadate_E2p(void)
{

    fn_dp = E2P_ReadLenByte(FN_DP_ADD, 4);          //数据发送频率
    fn_ang = E2P_ReadOneByte(FN_ANG_ADD);           //
    fn_freq = E2P_ReadOneByte(FN_FREQ_ADD);         //
    fn_pf = E2P_ReadOneByte(FN_PF_ADD);             //
    fn_lc = E2P_ReadLenByte(FN_LC_ADD, 4);          //
    fn_oc = E2P_ReadLenByte(FN_OC_ADD, 4);          //
    fn_ov = E2P_ReadLenByte(FN_OV_ADD, 4);          //
    fn_sag = E2P_ReadLenByte(FN_SAG_ADD, 4);        //
    fn_op = E2P_ReadLenByte(FN_OP_ADD, 4);          //
    fn_sw_e = E2P_ReadLenByte(FN_SW_E_ADD, 4);      //
    fn_sw_pc = E2P_ReadLenByte(FN_SW_PC_ADD, 4);    //
    cg_data_led = E2P_ReadOneByte(CG_DATA_LED_ADD); //
    de_sw_s = E2P_ReadOneByte(DE_SWITCH_STA_ADD);   //
    fn_sw_on = E2P_ReadLenByte(FN_SW_ON_ADD, 4);    //

    ESP_LOGI(TAG, "E2P USAGE:%d\n", E2P_USAGED);

    ESP_LOGI(TAG, "fn_dp:%d\n", fn_dp);
    ESP_LOGI(TAG, "fn_ang:%d\n", fn_ang);
    ESP_LOGI(TAG, "fn_freq:%d\n", fn_freq);
    ESP_LOGI(TAG, "fn_pf:%d\n", fn_pf);
    ESP_LOGI(TAG, "fn_lc:%d\n", fn_lc);
    ESP_LOGI(TAG, "fn_oc:%d\n", fn_oc);
    ESP_LOGI(TAG, "fn_ov:%d\n", fn_ov);
    ESP_LOGI(TAG, "fn_sag:%d\n", fn_sag);
    ESP_LOGI(TAG, "fn_op:%d\n", fn_op);
    ESP_LOGI(TAG, "fn_sw_e:%d\n", fn_sw_e);
    ESP_LOGI(TAG, "fn_sw_pc:%d\n", fn_sw_pc);
    ESP_LOGI(TAG, "cg_data_led:%d\n", cg_data_led);
    ESP_LOGI(TAG, "de_sw_s:%d\n", de_sw_s);
    ESP_LOGI(TAG, "fn_sw_on:%d\n", fn_sw_on);
}

void Read_Product_E2p(void)
{
    ESP_LOGI(TAG, "FIRMWARE=%s\n", FIRMWARE);
    E2P_Read(SERISE_NUM_ADDR, (uint8_t *)SerialNum, SERISE_NUM_LEN);
    ESP_LOGI(TAG, "SerialNum=%s\n", SerialNum);
    E2P_Read(PRODUCT_ID_ADDR, (uint8_t *)ProductId, PRODUCT_ID_LEN);
    ESP_LOGI(TAG, "ProductId=%s\n", ProductId);
    E2P_Read(WEB_HOST_ADD, (uint8_t *)WEB_SERVER, WEB_HOST_LEN);
    ESP_LOGI(TAG, "WEB_SERVER=%s\n", WEB_SERVER);
    E2P_Read(WEB_PORT_ADD, (uint8_t *)WEB_PORT, 5);
    ESP_LOGI(TAG, "WEB_PORT=%s\n", WEB_PORT);
    E2P_Read(MQTT_HOST_ADD, (uint8_t *)MQTT_SERVER, WEB_HOST_LEN);
    ESP_LOGI(TAG, "MQTT_SERVER=%s\n", MQTT_SERVER);
    E2P_Read(MQTT_PORT_ADD, (uint8_t *)MQTT_PORT, 5);
    ESP_LOGI(TAG, "MQTT_PORT=%s\n", MQTT_PORT);
    // E2P_Read(CHANNEL_ID_ADD, (uint8_t *)ChannelId, CHANNEL_ID_LEN);
    // ESP_LOGI(TAG, "ChannelId=%s\n", ChannelId);
    // E2P_Read(USER_ID_ADD, (uint8_t *)USER_ID, USER_ID_LEN);
    // ESP_LOGI(TAG, "USER_ID=%s\n", USER_ID);
    // E2P_Read(API_KEY_ADD, (uint8_t *)ApiKey, API_KEY_LEN);
    // ESP_LOGI(TAG, "ApiKey=%s\n", ApiKey);
    E2P_Read(WIFI_SSID_ADD, (uint8_t *)wifi_data.wifi_ssid, sizeof(wifi_data.wifi_ssid));
    E2P_Read(WIFI_PASSWORD_ADD, (uint8_t *)wifi_data.wifi_pwd, sizeof(wifi_data.wifi_pwd));
    ESP_LOGI(TAG, "wifi ssid=%s,wifi password=%s\n", wifi_data.wifi_ssid, wifi_data.wifi_pwd);

    if (strlen(WEB_SERVER) == 0)
    {
        snprintf(WEB_SERVER, sizeof(WEB_SERVER), "api.ubibot.cn");
    }
    if (strlen(WEB_PORT) == 0)
    {
        snprintf(WEB_PORT, sizeof(WEB_PORT), "80");
    }
    if (strlen(MQTT_SERVER) == 0)
    {
        snprintf(MQTT_SERVER, sizeof(MQTT_SERVER), "mqtt.ubibot.cn");
    }
    if (strlen(MQTT_PORT) == 0)
    {
        snprintf(MQTT_PORT, sizeof(MQTT_PORT), "1883");
    }
}

void Read_Fields_E2p(void)
{
    uint8_t sensors_temp;

    if ((sensors_temp = E2P_ReadOneByte(F_SW_S_ADD)) != 0)
    {
        f_sw_s = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_V_ADD)) != 0)
    {
        f_sw_v = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_C_ADD)) != 0)
    {
        f_sw_c = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_P_ADD)) != 0)
    {
        f_sw_p = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_LC_ADD)) != 0)
    {
        f_sw_lc = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_PC_ADD)) != 0)
    {
        f_sw_pc = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_RSSI_W_ADD)) != 0)
    {
        f_rssi_w = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_ON_ADD)) != 0)
    {
        f_sw_on = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_ANG_ADD)) != 0)
    {
        f_sw_ang = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_S_FREQ_ADD)) != 0)
    {
        f_sw_freq = sensors_temp;
    }
    if ((sensors_temp = E2P_ReadOneByte(F_SW_PF_ADD)) != 0)
    {
        f_sw_pf = sensors_temp;
    }

    // for (uint8_t i = 0; i < 40; i++)
    // {
    //     f_cali_u[i].dat = E2P_ReadLenByte(5 * i + F1_A_ADDR, 4);
    //     if (i % 2 == 0 && f_cali_u[i].val == 0)
    //     {
    //         f_cali_u[i].val = 1;
    //     }

    //     ESP_LOGI(TAG, "%s:%f\n", f_cali_str[i], f_cali_u[i].val);
    // }

    ESP_LOGI(TAG, "f_sw_s:%d\n", f_sw_s);
    ESP_LOGI(TAG, "f_sw_v:%d\n", f_sw_v);
    ESP_LOGI(TAG, "f_sw_c:%d\n", f_sw_c);
    ESP_LOGI(TAG, "f_sw_p:%d\n", f_sw_p);
    ESP_LOGI(TAG, "f_sw_lc:%d\n", f_sw_lc);
    ESP_LOGI(TAG, "f_sw_pc:%d\n", f_sw_pc);
    ESP_LOGI(TAG, "f_rssi_w:%d\n", f_rssi_w);
    ESP_LOGI(TAG, "f_sw_on:%d\n", f_sw_on);
    ESP_LOGI(TAG, "f_sw_ang:%d\n", f_sw_ang);
    ESP_LOGI(TAG, "f_sw_freq:%d\n", f_sw_freq);
    ESP_LOGI(TAG, "f_sw_pf:%d\n", f_sw_pf);
}