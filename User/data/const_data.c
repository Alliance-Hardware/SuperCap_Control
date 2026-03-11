#include "const_data.h"

// 设备固有常量定义
const uint32_t SUPERCAP_ID[4][3] = {
    {0x12345678, 0x9ABCDEF0, 0x13579BDF},
    {0x001A0044, 0x59315007, 0x20393448},
    {0x32345678, 0x9ABCDEF0, 0x13579BDF},
    {0X001A003C, 0X59315007, 0X20393448}};  // Supercap 设备ID//测试板的ID

const int DEFAULT_POWER_CHASSIS = 45;  // 初始默认底盘功率（单位：W）

// ADC校准参数定义
const float ADC_CALIBRATION_CONFIGS[4][4][2] = {
    {{1.0, 0.0}, {1.0, 0.0}, {1.0, 0.0}, {1.0, 0.0}},        // Board 0
    {{1.1, -5.0}, {0.9, 3.0}, {1.05, -2.0}, {0.95, 1.0}},    // Board 1
    {{0.95, 4.0}, {1.05, -3.0}, {0.98, 2.0}, {1.02, -1.0}},  // Board 2
    {{1.02, -2.0}, {0.98, 2.0}, {1.03, -1.0}, {0.97, 3.0}}   // Board 3
};

// 底盘工作电压范围
const float V_CHASSIS_MAX = 26.0f;
const float V_CHASSIS_MIN = 22.0f;

// 底盘工作功率范围
const float P_CHASSIS_MAX = 120.0f;
const float P_CHASSIS_MIN = 35.0f;  // 底盘能量消耗至0后，机器人进入节能模式 35W

// 超级电容工作电压、电流范围
const float V_CAP_MAX = 26.0f;
const float V_CAP_MIN = 4.0f;
const float I_CAP_MAX = 10.0f;
const float I_CAP_MIN = -10.0f;
// 开关管驱动参数
const uint32_t CYCLE_ZERO = 0;
const uint32_t CYCLE_INDEX = 27200;
const uint32_t HALF_CYCLE_INDEX = CYCLE_INDEX / 2;
const float DUTY_INDEX = 0.9f;
const float MAX_DUTY = 0.53f;  // 13.0f / 24.0f最大占空比限制
const float MIN_DUTY = 0.01f;  // 最小占空比限制0.01f,用于缓启动

// 保护机制时间常数定义
const int MAX_POWER_ERROR_DETECTION_TIME = 500;     // 最大底盘电压异常检测时间
const int MAX_CAN_DISCONNECT_DETECTION_TIME = 500;  // 最大CAN断联检测时间
int POWER_ERROR_DETECTION_TIME_INDEX = 0;     // 底盘电压异常检测时间计数变量
int CAN_DISCONNECT_DETECTION_TIME_INDEX = 0;  // CAN断联检测时间计数变量
uint32_t PID_FREQUENCY_INDEX = 0;             // PID控制频率计数变量
