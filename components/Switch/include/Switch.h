
#ifndef _SWITCH_H_
#define _SWITCH_H_

#include "freertos/event_groups.h"
#include "freertos/task.h"

#define M_IN1 19  //电机驱动IO 1
#define M_IN2 22  //电机驱动IO 2
#define EN_TRIP 4 //脱扣器
#define HALL_M 36 //电机齿轮
#define HALL_S 38 //闸门
#define HALL_F 37 //保险

#define EN_TEST 21 //漏电检测使能

#define TEST_EN_TIME 100 * 1000    //us
#define TRIP_EN_TIME 100 * 1000    //us
#define MOTOR_OUT_TIME 1000 * 1000 //us

EventGroupHandle_t Sw_sta_group;
#define SW_STA_BIT (1 << 0) //闸门状态
#define M_STA_BIT (1 << 1)  //电机复位状态

extern bool sw_sta; //1 合闸
extern bool f_sta;  //0 复位
extern bool m_sta;  //0 复位

void Switch_Init(void);
void Switch_Relay(int8_t set_value);
void Start_Leak_Test(void);

#endif
