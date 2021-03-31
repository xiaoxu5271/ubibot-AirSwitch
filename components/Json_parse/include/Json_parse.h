#ifndef __Json_parse
#define __Json_parse
#include <stdio.h>
#include "esp_err.h"
#include "E2prom.h"

esp_err_t parse_objects_http_active(char *http_json_data);
esp_err_t parse_objects_bluetooth(char *blu_json_data);
esp_err_t parse_objects_mqtt(char *mqtt_json_data, bool sw_flag);
esp_err_t parse_objects_heart(char *json_data);
esp_err_t parse_objects_http_respond(char *http_json_data);
esp_err_t ParseTcpUartCmd(char *pcCmdBuffer);

esp_err_t creat_object(void);

#define NET_AUTO 2 //上网模式 自动
#define NET_4G 1   //上网模式 4G
#define NET_WIFI 0 //上网模式 wifi

#define FILED_BUFF_SIZE 350
#define CALI_BUFF_SIZE 512

struct
{
    bool mqtt_switch_status;
    char mqtt_command_id[32];
    char mqtt_ota_url[128]; //OTA升级地址
    uint32_t mqtt_file_size;
} mqtt_json_s;

struct
{
    char wifi_ssid[32];
    char wifi_pwd[64];
} wifi_data;

struct
{
    char http_time[24];
} http_json_c;

typedef struct
{
    char buff[256];
    uint8_t len;
} creat_json;

typedef union
{
    float val;
    uint32_t dat;
} f_cali;

// enum
// {
//     loop = 0,
//     physica,
//     console,
//     app,
//     alexa
// } c_type;

//creat_json *create_http_json(uint8_t post_status);
void create_http_json(creat_json *pCreat_json, uint8_t flag);
void Read_Metadate_E2p(void);
void Read_Product_E2p(void);
void Read_Fields_E2p(void);
void Create_NET_Json(void);
void Create_Switch_Json(void);
uint16_t Create_Status_Json(char *status_buff, uint16_t buff_len, bool filed_flag);
char *mid(char *src, char *s1, char *s2, char *sub);
char *s_rstrstr(const char *_pBegin, int _MaxLen, int _ReadLen, const char *_szKey);
char *s_rstrstr_2(const char *_pBegin, int _MaxLen, const char *_szKey);
char *s_strstr(const char *_pBegin, int _ReadLen, int *first_len, const char *_szKey);
double Cali_filed(uint8_t filed_num, double filed_val);
void Create_cali_buf(char *read_buf);

//metadata 相关变量
// uint8_t fn_set_flag = 0;     //metadata 读取标志，未读取则使用下面固定参数
extern uint32_t fn_dp;      //数据发送频率
extern uint8_t fn_ang;      //相位角测量，0关闭，1开启
extern uint8_t fn_freq;     //交流电频率测量，0关闭，1开启
extern uint8_t fn_pf;       //功率因数测量，0关闭，1开启
extern uint32_t fn_lc;      //漏电保护设置 10-100 ，0关闭
extern uint32_t fn_oc;      //过流保护设置，0关闭
extern uint32_t fn_ov;      //过压保护设置，0关闭
extern uint32_t fn_sag;     //欠压保护设置，0关闭
extern uint32_t fn_op;      //过载保护设置，0关闭
extern uint32_t fn_sw_e;    //电能信息采集周期：电压/电流/功率
extern uint32_t fn_sw_pc;   //用电量统计
extern uint8_t cg_data_led; //发送数据 LED状态 0关闭，1打开
extern uint8_t de_sw_s;     //开关默认上电状态
extern uint32_t fn_sw_on;   //开启时长统计

// field num 相关参数
extern uint8_t f_sw_s;    //开关状态 0关闭，1开启
extern uint8_t f_sw_v;    //电压
extern uint8_t f_sw_c;    //电流
extern uint8_t f_sw_p;    //功率
extern uint8_t f_sw_lc;   //漏电电流
extern uint8_t f_sw_pc;   //累计用电量
extern uint8_t f_rssi_w;  //WIFI 信号
extern uint8_t f_sw_on;   //累计开启时长
extern uint8_t f_sw_ang;  //相位角
extern uint8_t f_sw_freq; //市电频率
extern uint8_t f_sw_pf;   //功率因数

//cali 相关
extern f_cali fn_val[40];

extern char SerialNum[SERISE_NUM_LEN];
extern char ProductId[PRODUCT_ID_LEN];
extern char ApiKey[API_KEY_LEN];
extern char ChannelId[CHANNEL_ID_LEN];
extern char USER_ID[USER_ID_LEN];
extern char WEB_SERVER[WEB_HOST_LEN];
extern char MQTT_SERVER[WEB_HOST_LEN];
extern char WEB_PORT[5];
extern char MQTT_PORT[5];
extern char BleName[100];
// extern char SIM_APN[32];
// extern char SIM_USER[32];
// extern char SIM_PWD[32];

//c-type
extern char C_TYPE[10];

#endif