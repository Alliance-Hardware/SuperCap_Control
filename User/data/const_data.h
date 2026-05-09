#pragma once
#include <stdint.h>

#include "module_data.h"

// 设备固有常量定义
#define RMCS_ID 0x1FE                     // RMCS 设备ID
#define LEGGED_ID 0x02                    // Legged 设备ID
extern const uint32_t SUPERCAP_ID[4][3];  // Supercap 设备ID//测试板的ID
extern const int DEFAULT_POWER_CHASSIS;   // 初始默认底盘功率（单位：W）
// ADC校准参数定义
extern const float ADC_CALIBRATION_CONFIGS[4][4][2];
// I_CAP_Scale, I_CAP_Bias} // ADC校准参数配置数组

// 底盘工作电压范围
extern const float V_CHASSIS_MAX;
extern const float V_CHASSIS_MIN;
extern const float P_CHASSIS_MAX;
extern const float P_CHASSIS_MIN;  // 底盘能量消耗至0后，机器人进入节能模式 35W

// 超级电容工作电压、电流范围
extern const float V_CAP_MAX;
extern const float V_CAP_MIN;
extern const float I_CAP_MAX;
extern const float I_CAP_MIN;

// 开关管驱动参数
extern const uint32_t CYCLE_ZERO;
extern const uint32_t CYCLE_INDEX;
extern const uint32_t HALF_CYCLE_INDEX;
extern const float DUTY_INDEX;
// 由于半桥没有办法真正意义常开。缩短实际占空比时间用于自举电容充电，此处为缩短使用的占空比系数
extern const float MAX_DUTY;
extern const float MIN_DUTY;
extern const float V_CAP_FULL;
extern const float ALPHA;

// 保护机制时间常数定义
extern const int MAX_POWER_ERROR_DETECTION_TIME;     // 最大底盘电压异常检测时间
extern const int MAX_CAN_DISCONNECT_DETECTION_TIME;  // 最大CAN断联检测时间
extern int POWER_ERROR_DETECTION_TIME_INDEX;     // 底盘电压异常检测时间计数变量
extern int CAN_DISCONNECT_DETECTION_TIME_INDEX;  // CAN断联检测时间计数变量
extern uint32_t PID_FREQUENCY_INDEX;             // PID控制频率计数变量
