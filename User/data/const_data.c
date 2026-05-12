#include "const_data.h"

// 设备固有常量定义
const uint32_t SUPERCAP_ID[4][3] = {
    {0x00040015, 0x3335510A, 0x39323936},
    {0x003E0035, 0x3335510A, 0x39323936},
    {0x00320016, 0x3335510A, 0x39323936},
    {0x00450047, 0x3330500C, 0x20383352}};  // Supercap 设备ID//测试板的ID

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
const float V_CHASSIS_MIN = 20.0f;

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
const float MAX_DUTY =
    0.565f;  // buck-boost: D=Vout/(Vin+Vout), Vin=20V, Vout=26V
const float MIN_DUTY = 0.01f;    // 最小占空比限制0.01f,用于缓启动'
const float V_CAP_FULL = 26.0f;  // 电容安全满电电压
const float ALPHA = 0.1f;        // 低通滤波器系数

// 保护机制时间常数定义
const int MAX_POWER_ERROR_DETECTION_TIME = 1000;     // 最大底盘电压异常检测时间
const int MAX_CAN_DISCONNECT_DETECTION_TIME = 5000;  // 最大CAN断联检测时间
int POWER_ERROR_DETECTION_TIME_INDEX = 0;     // 底盘电压异常检测时间计数变量
int CAN_DISCONNECT_DETECTION_TIME_INDEX = 0;  // CAN断联检测时间计数变量
uint32_t PID_FREQUENCY_INDEX = 0;             // PID控制频率计数变量
