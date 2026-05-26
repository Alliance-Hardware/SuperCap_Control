#include "module_data.h"
// CAN_communicate模块的数据结构体实例化
CAN_TX can_tx;
CAN_RX can_rx;
FDCAN_RxHeaderTypeDef rx_header;
uint8_t can_rx_data[8] = {0};
uint8_t can_tx_data[8] = {0};

// CAN过滤器配置结构体
FDCAN_FilterTypeDef fdcan_filter_config = {
    .IdType = FDCAN_STANDARD_ID,
    .FilterIndex = 0,
    .FilterType = FDCAN_FILTER_MASK,
    .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
    .FilterID1 = 0x000,   // ID
    .FilterID2 = 0x000};  // Mask，全 0 表示不过滤任何位

// CAN发送消息头配置
FDCAN_TxHeaderTypeDef fdcan_tx_header = {
    .Identifier = 0X300,  // 假设这个是超电的id
    .IdType = FDCAN_STANDARD_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
    .DataLength = FDCAN_DLC_BYTES_8,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_CLASSIC_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0};

// MOS_driver模块的数据结构体实例化
mosdriver mos_driver;

// Data_collect模块的数据结构体实例化
datacollect adc_data;

// PID_controller模块的数据结构体实例化
PID_Configs voltage_pid_configs;  // 电压环PID配置结构体全局定义
PID_Configs current_pid_configs;  // 电流环PID配置结构体全局定义
PID_Configs power_pid_configs;    // 功率环PID配置结构体全局定义
float chassis_power;              // 当前底盘功率
float dynamic_max_duty;           // 动态最大占空比
float dynamic_max_duty_pre;       // 上一次动态最大占空比
float chassis_voltage_window;     // 底盘电压窗口滤波值
// 底盘电压窗口滤波相关变量
float chassis_voltage_window_buf[10] = {0};
uint8_t chassis_voltage_window_idx = 0;

// ADC校准配置数组实例化
float ADC_CALIBRATION_CONFIGS_BOARD[4][2] = {
    {0.0, 0.0},  // V_CHASSIS
    {0.0, 0.0},  // I_CHASSIS
    {0.0, 0.0},  // V_CAP
    {0.0, 0.0}   // I_CAP
};

int last_can_enable = 1;