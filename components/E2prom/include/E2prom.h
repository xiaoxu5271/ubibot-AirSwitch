
#ifndef _E2PROM_H_
#define _E2PROM_H_

#include "freertos/FreeRTOS.h"

#if 1
#define I2C_MASTER_SCL_IO 14     /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 27     /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1 /*!< I2C port number for master dev */

#else
#define I2C_MASTER_SCL_IO 19     /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 18     /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1 /*!< I2C port number for master dev */

#endif

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

// #define FM24
//Device Address
#define FE_E2P_SIZE 8 * 1024
#define FE_DEV_ADD 0XAE

#define AT_E2P_SIZE 1024
#define AT_DEV_ADD 0XA8

#define PRODUCT_ID_LEN 32
#define SERISE_NUM_LEN 16
#define WEB_HOST_LEN 32
#define CHANNEL_ID_LEN 16
#define USER_ID_LEN 48
#define API_KEY_LEN 33

//参数地址
#define PRODUCT_ID_ADDR 0                                    //product id
#define SERISE_NUM_ADDR PRODUCT_ID_ADDR + PRODUCT_ID_LEN + 1 //serise num
#define WEB_HOST_ADD SERISE_NUM_ADDR + SERISE_NUM_LEN + 1    //web host
#define CHANNEL_ID_ADD WEB_HOST_ADD + WEB_HOST_LEN + 1       //chanel id
#define USER_ID_ADD CHANNEL_ID_ADD + CHANNEL_ID_LEN + 1      //user id
#define API_KEY_ADD USER_ID_ADD + USER_ID_LEN + 1            //user id
#define WEB_PORT_ADD API_KEY_ADD + API_KEY_LEN + 1           //web PORT
#define MQTT_HOST_ADD WEB_PORT_ADD + 5 + 1                   //MQTT HOST
#define MQTT_PORT_ADD MQTT_HOST_ADD + WEB_HOST_LEN + 1       //MQTT PORT

//metadata
#define FN_SET_FLAG_ADD MQTT_PORT_ADD + 5 + 1 //metadata setted flag u8
#define FN_DP_ADD FN_SET_FLAG_ADD + 1 + 1     //数据发送频率uint32_t
#define FN_ANG_ADD FN_DP_ADD + 4 + 1          //4
#define FN_FREQ_ADD FN_ANG_ADD + 4 + 1        //4
#define FN_PF_ADD FN_FREQ_ADD + 4 + 1         //4
#define FN_LC_ADD FN_PF_ADD + 4 + 1           //4
#define FN_OC_ADD FN_LC_ADD + 4 + 1           //485温湿度探头uint32_t
#define FN_OV_ADD FN_OC_ADD + 4 + 1           //485 土壤探头uint32_t
#define FN_SAG_ADD FN_OV_ADD + 4 + 1
#define FN_OP_ADD FN_SAG_ADD + 4 + 1  //18b20uint32_t
#define FN_SW_E_ADD FN_OP_ADD + 4 + 1 //电能信息：电压/电流/功率uint32_t
#define FN_SW_PC_ADD FN_SW_E_ADD + 4 + 1
#define CG_DATA_LED_ADD FN_SW_PC_ADD + 4 + 1      //发送数据 LED状态 0关闭，1打开uint8_t
#define DE_SWITCH_STA_ADD CG_DATA_LED_ADD + 1 + 1 //uint8_t
#define FN_SW_ON_ADD DE_SWITCH_STA_ADD + 1 + 1    //4

//sensors
#define F_SW_S_ADD FN_SW_ON_ADD + 4 + 1
#define F_SW_V_ADD F_SW_S_ADD + 1 + 1
#define F_SW_C_ADD F_SW_V_ADD + 1 + 1
#define F_SW_P_ADD F_SW_C_ADD + 1 + 1
#define F_SW_LC_ADD F_SW_P_ADD + 1 + 1
#define F_SW_PC_ADD F_SW_LC_ADD + 1 + 1
#define F_RSSI_W_ADD F_SW_PC_ADD + 1 + 1
#define F_SW_ON_ADD F_RSSI_W_ADD + 1 + 1
#define F_SW_ANG_ADD F_SW_ON_ADD + 1 + 1
#define F_SW_S_FREQ_ADD F_SW_ANG_ADD + 1 + 1
#define F_SW_PF_ADD F_SW_S_FREQ_ADD + 1 + 1

#define LAST_SWITCH_ADD F_SW_PF_ADD + 1 + 1 //UINT8

#define WIFI_SSID_ADD LAST_SWITCH_ADD + 1 + 1    //32
#define WIFI_PASSWORD_ADD WIFI_SSID_ADD + 32 + 1 //64

#define E2P_USAGED WIFI_PASSWORD_ADD + 64 + 1

void E2prom_Init(void);
esp_err_t E2P_WriteOneByte(uint16_t reg_addr, uint8_t dat);
uint8_t E2P_ReadOneByte(uint16_t reg_addr);
void E2P_WriteLenByte(uint16_t WriteAddr, uint32_t DataToWrite, uint8_t Len);
uint32_t E2P_ReadLenByte(uint16_t ReadAddr, uint8_t Len);
void E2P_Read(uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);
void E2P_Write(uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite);
void E2prom_empty_all(bool flag);
void E2prom_set_defaul(bool flag);

#endif
