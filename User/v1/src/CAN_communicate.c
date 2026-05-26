#include "CAN_communicate.h"

#include "MOS_driver.h"
#include "SuperCap_init.h"
#include "const_data.h"
#include "module_data.h"

void CAN_init() {
    can_rx.targetChassisPower = 0;
    can_rx.enabled = 1;

    can_tx.chassis_power = 0;
    can_tx.supercap_voltage = 0;
    can_tx.chassis_voltage = 0;
    can_tx.enabled = 1;
    can_tx.unused = 1;
    // 开启CAN接收过滤器
    HAL_FDCAN_ConfigFilter(&hfdcan2, &fdcan_filter_config);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE,
                                 FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_Start(&hfdcan2);
}

static uint16_t double_to_uint16(double x, double min, double max) {
    if (x < min)
        x = min;
    else if (x > max)
        x = max;

    const double span = max - min;
    if (span <= 0.0) return 0;

    const double scale = 65535.0;  // uint16_t 满量程
    return (uint16_t)((x - min) * scale / span + 0.5);
}

void CAN_send() {
    // 按接收端映射反向编码
    can_tx.chassis_power = double_to_uint16(chassis_power, -100.0, 400.0);
    can_tx.supercap_voltage = double_to_uint16(adc_data.V_CAP_TF, 0.0, 50.0);
    can_tx.chassis_voltage = double_to_uint16(adc_data.V_CHASSIS_TF, 0.0, 50.0);

    if (can_rx.enabled) {
        can_tx.enabled = 1;
        can_tx.unused = 1;
    } else {
        can_tx.enabled = 0;
        can_tx.unused = 0;
    }
    // 小端序发送（低字节在前），与接收端 bit_cast 结构体一致
    can_tx_data[0] = (uint8_t)(can_tx.chassis_power & 0xFF);
    can_tx_data[1] = (uint8_t)(can_tx.chassis_power >> 8);
    can_tx_data[2] = (uint8_t)(can_tx.supercap_voltage & 0xFF);
    can_tx_data[3] = (uint8_t)(can_tx.supercap_voltage >> 8);
    can_tx_data[4] = (uint8_t)(can_tx.chassis_voltage & 0xFF);
    can_tx_data[5] = (uint8_t)(can_tx.chassis_voltage >> 8);
    can_tx_data[6] = (can_tx.enabled);
    can_tx_data[7] = (can_tx.unused);
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &fdcan_tx_header, can_tx_data);
    if (CAN_DISCONNECT_DETECTION_TIME_INDEX >=
        MAX_CAN_DISCONNECT_DETECTION_TIME) {
        // MosDriver_stop(&mos_driver);
        // can_tx.enabled = 0;
        // 超过最大CAN断连检测时间，认为CAN断连，关闭半桥
        CAN_DISCONNECT_DETECTION_TIME_INDEX = MAX_CAN_DISCONNECT_DETECTION_TIME;
        // 防止溢出
    }
    CAN_DISCONNECT_DETECTION_TIME_INDEX++;
    // CAN断连检测计数变量自增,正常情况下，CAN断连检测计数变量会在收到信息后重置，根据CAN发送频率1KHz,最多500ms未收到信息则判定为断连
}

void CAN_receive() {
    if (HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &rx_header,
                               can_rx_data) == HAL_OK) {
        if (rx_header.Identifier == RMCS_ID) {
            can_rx.targetChassisPower = can_rx_data[6];
            can_rx.enabled = can_rx_data[7];
            // can信息有效性检查
            if (can_rx.enabled != 1 && can_rx.enabled != 0) {
                // MosDriver_stop(&mos_driver);
            }
            if (can_rx.targetChassisPower > P_CHASSIS_MAX) {
                can_rx.targetChassisPower = P_CHASSIS_MAX;
            } else if (can_rx.targetChassisPower < P_CHASSIS_MIN) {
                can_rx.targetChassisPower = P_CHASSIS_MIN;
            }
            if (can_rx.enabled == 0) {
                last_can_enable = 0;
                MosDriver_stop(&mos_driver);
            }
            if (can_rx.enabled == 1 && last_can_enable == 0) {
                // 重新初始化MOS驱动和PID，允许重启
                MosDriver_init(&mos_driver);
                MosDriver_TIMER_init();
                PID_init(&current_pid_configs);
                PID_init(&voltage_pid_configs);
                PID_init(&power_pid_configs);
                last_can_enable = 1;
                PID_set(&current_pid_configs, 0.002f, 0.00002f, 0.0f, MAX_DUTY,
                        MIN_DUTY, true);
                // 电流环PID参数设置
                PID_set(&voltage_pid_configs, 0.0f, 0.0f, 0.0f, V_CAP_MAX,
                        V_CAP_MIN, false);
                // 电压环PID参数设置
                PID_set(&power_pid_configs, 0.04f, 0.006f, 0.0f, I_CAP_MAX,
                        I_CAP_MIN, true);
                float V_CHASSIS_INIT = adc_data.V_CHASSIS_ADC;
                // 读取底盘电压ADC值
                float V_CAP_INIT = adc_data.V_CAP_ADC;
                // 读取电容组电压ADC值
                //  这里的其实ADC/转换值都行，只是ADC值计算更简单一些
                float general_duty = V_CAP_INIT / (V_CHASSIS_INIT + V_CAP_INIT);
                // 计算软启动占空比，这里是基于BUCK-BOOST控制模式进行计算
                MosDriver_dutylimit(&mos_driver, general_duty);
                // 避免超过最大占空比
                current_pid_configs.output = general_duty;
                MosDriver_OUTPUT_init();
            }
            power_pid_configs.target_value = can_rx.targetChassisPower;
            // 避免上位机发送功率错误导致的功率控制异常
        }
    }
    CAN_DISCONNECT_DETECTION_TIME_INDEX =
        0;  // 收到信息后重置CAN断连检测计数变量
}

void CAN_disconnect_detection() {}
