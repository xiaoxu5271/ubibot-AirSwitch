#ifndef __HLW8110_H
#define __HLW8110_H

#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

//IO端口定义
#define HLW_SDO 33
#define HLW_SDI 5
#define HLW_SCLK 18
#define HLW_SCSN 23
#define HLW_INT1 25
#define HLW_INT2 26

//8112/8110 reg define
#define REG_SYSCON_ADDR 0x00
#define REG_EMUCON_ADDR 0x01
#define REG_HFCONST_ADDR 0x02
#define REG_EMUCON2_ADDR 0x13
#define REG_ANGLE_ADDR 0x22 //相角寄存器
#define REG_UFREQ_ADDR 0x23 //市电线性频率地址
#define REG_RMSIA_ADDR 0x24
#define REG_RMSIB_ADDR 0x25
#define REG_RMSU_ADDR 0x26
#define REG_PF_ADDR 0x27
#define REG_ENERGY_PA_ADDR 0x28
#define REG_ENERGY_PB_ADDR 0x29
#define REG_POWER_PA_ADDR 0x2C
#define REG_POWER_PB_ADDR 0x2D

#define REG_OVLVL_ADDR 0x19
#define REG_OIBLVL_ADDR 0X1B

#define REG_INT_ADDR 0x1D
#define REG_IE_ADDR 0x40
#define REG_IF_ADDR 0x41
#define REG_RIF_ADDR 0x42

#define REG_RDATA_ADDR 0x44

#define REG_CHECKSUM_ADDR 0x6f
#define REG_RMS_IAC_ADDR 0x70
#define REG_RMS_IBC_ADDR 0x71
#define REG_RMS_UC_ADDR 0x72
#define REG_POWER_PAC_ADDR 0x73
#define REG_POWER_PBC_ADDR 0x74
#define REG_POWER_SC_ADDR 0x75
#define REG_ENERGY_AC_ADDR 0x76
#define REG_ENERGY_BC_ADDR 0x77

#define D_TIME1_50MS 50

//直流校正系数

//8112A通道或8110通道校正系数
#define D_CAL_U 1000 / 1000   //电压校正系数
#define D_CAL_A_I 1           //A通道电流校正系数
#define D_CAL_A_P 1000 / 1000 //A通道功率校正系数
#define D_CAL_A_E 1000 / 1000 //A通道电能校正系数

//8112 B通道校正系数
#define D_CAL_B_P 1000 / 1000 //B通道功率校正系数
#define D_CAL_B_I 1           //B通道电流校正系数
#define D_CAL_B_E 1000 / 1000 //B通道电能校正系数

//免校准系数
#define K_A_I 6   //A通道电流
#define K_A_U 1   //A通道电压
#define K_A_P 1   //A通道功率
#define K_B_I 200 //B通道电流
#define K_A_U 1   //B通道电压

//用于计算
#define L_I_reg 752100 // 实际B通道电流寄存器值
#define L_I 0.023      //实际施加B通道电流

TaskHandle_t HLW_Read_Task_Hanlde;

void HLW_Init(void);

#endif
