#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "const_data.h"
#include "main.h"
#include "stm32g4xx_hal_fdcan.h"

// 控制算法开关（放在枚举之外，作为独立的常量）
#define control_v1 0  // 控制算法v1
#define control_v2 0  // 控制算法v2

/*这里有两个版本是由于版本一是洪泓霖学长设计的控制算法，版本二是我
新设计的控制算法，本套代码主要目的是重构洪泓霖学长的代码，提供更好
的调试框架，其次才是我考虑实现我的控制算法，具体这两种控制算法的设
计可以查看下面这个链接:
https://fa4g5no1b1f.feishu.cn/wiki/AhU2wcN2ditQpXkVa6RcQfOXnyc

// */

// 存放超级电容相关的运行状态枚举
enum State {
    DCDC_OUTPUT_DISABLE = 0,  // DCDC 输出关闭
    DCDC_OUTPUT_ENABLE = 1,   // DCDC 输出使能
};  // enum State

// CAN_communicate模块的数据结构体定义
typedef struct {
    uint16_t chassis_power;     // 当前底盘功率
    uint16_t supercap_voltage;  // 当前超级电容电压
    uint16_t chassis_voltage;   // 当前底盘电压
    uint8_t enabled;            // 是否开启栅极驱动
    uint8_t unused;             // 标志位，超级电容组是否被使用
} CAN_TX;

typedef struct {
    uint16_t targetChassisPower;  // 目标底盘功率
    uint8_t enabled;              // 是否开启栅极驱动
} CAN_RX;

// MOS_driver模块的数据结构体定义
typedef struct {
    uint32_t cap_compare1_index;
    uint32_t cap_compare3_index;
    uint32_t chassis_compare1_index;
    uint32_t chassis_compare3_index;
    uint32_t compare4_index;  // 触发采样的比较值

    uint32_t Phase_shift_angle;  // 相位移角度
} mosdriver;

// Data_collect模块的数据结构体定义
typedef struct {
    uint16_t DataArray[4];  // ADC采样数组

    uint16_t V_CHASSIS_ADC;  // 底盘端电压->ADC值
    uint16_t I_CHASSIS_ADC;  // 底盘端电流->ADC值
    uint16_t I_CAP_ADC;      // 电容组端电流->ADC值
    uint16_t V_CAP_ADC;      // 电容组端电压->ADC值

    float V_CHASSIS_TF;  // 底盘端电压->转换值
    float I_CHASSIS_TF;  // 底盘端电流->转换值
    float V_CAP_TF;      // 电容组端电压->转换值
    float I_CAP_TF;      // 电容组端电流->转换值
} datacollect;

// PID_controller模块的数据结构体定义
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float error;
    float pre_error;
    float pre_pre_error;
    float output;
    float OUT_MAX;
    float OUT_MIN;
    float target_value;
    bool SWITCH;
} PID_Configs;

// 全局变量声明
// CAN通信相关全局变量
extern CAN_TX can_tx;                            // CAN发送数据结构体
extern CAN_RX can_rx;                            // CAN接收数据结构体
extern FDCAN_FilterTypeDef fdcan_filter_config;  // CAN过滤器配置结构体
extern FDCAN_TxHeaderTypeDef fdcan_tx_header;    // CAN发送消息头配置结构体
extern FDCAN_RxHeaderTypeDef rx_header;          // CAN接收消息头配置结构体
extern uint8_t can_rx_data[8];                   // CAN接收数据数组
extern uint8_t can_tx_data[8];                   // CAN发送数据数组

// MOS驱动相关全局变量
extern mosdriver mos_driver;  // MOS驱动数据结构体

// ADC采集相关全局变量
extern datacollect adc_data;             // ADC采集数据结构体
extern datacollect adc_calibrated_data;  // ADC校准后数据结构体
extern float ADC_CALIBRATION_CONFIGS_BOARD[4][2];

// PID控制相关全局变量
extern PID_Configs current_pid_configs;  // 电流环PID配置结构体
extern PID_Configs voltage_pid_configs;  // 电压环PID配置结构体
extern PID_Configs power_pid_configs;    // 功率环PID配置结构体
extern float chassis_power;              // 当前底盘功率