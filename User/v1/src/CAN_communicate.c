#include "CAN_communicate.h"

#include "MOS_driver.h"
#include "const_data.h"
#include "module_data.h"

void CAN_init() {
    can_rx.targetChassisPower = 0;
    can_rx.enabled = 0;

    can_tx.chassis_power = 0;
    can_tx.supercap_voltage = 0;
    can_tx.chassis_voltage = 0;
    can_tx.enabled = 0;
    can_tx.unused = 0;
    // 开启CAN接收过滤器
    HAL_FDCAN_ConfigFilter(&hfdcan2, &fdcan_filter_config);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE,
                                 FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_Start(&hfdcan2);
}

void CAN_send() {
    can_tx.chassis_power = (uint16_t)(chassis_power * 10);
    // 将底盘功率放大10倍，保留一位小数
    can_tx.supercap_voltage = (uint16_t)adc_data.V_CAP_TF;
    can_tx.chassis_voltage = (uint16_t)adc_data.V_CHASSIS_TF;
    if (can_rx.enabled) {
        can_tx.enabled = 1;
        can_tx.unused = 1;
    } else {
        can_tx.enabled = 0;
        can_tx.unused = 0;
    }
    can_tx_data[0] = (can_tx.chassis_power >> 8);
    can_tx_data[1] = (can_tx.chassis_power & 0xFF);
    can_tx_data[2] = (can_tx.supercap_voltage >> 8);
    can_tx_data[3] = (can_tx.supercap_voltage & 0xFF);
    can_tx_data[4] = (can_tx.chassis_voltage >> 8);
    can_tx_data[5] = (can_tx.chassis_voltage & 0xFF);
    can_tx_data[6] = (can_tx.enabled);
    can_tx_data[7] = (can_tx.unused);
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &fdcan_tx_header, can_tx_data);
    if (CAN_DISCONNECT_DETECTION_TIME_INDEX >=
        MAX_CAN_DISCONNECT_DETECTION_TIME) {
        // MosDriver_stop(&mos_driver);
        can_tx.enabled = 0;
        // 超过最大CAN断连检测时间，认为CAN断连，关闭半桥
        CAN_DISCONNECT_DETECTION_TIME_INDEX = MAX_CAN_DISCONNECT_DETECTION_TIME;
        // 防止溢出
    }
    CAN_DISCONNECT_DETECTION_TIME_INDEX++;
    // CAN断连检测计数变量自增,正常情况下，CAN断连检测计数变量会在收到信息后重置，根据CAN发送频率1KHz,最多500ms未收到信息则判定为断连
}

void CAN_receive() {
    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) > 0) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &rx_header,
                               can_rx_data);
        if (rx_header.Identifier == 0X01) {
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
            // 避免上位机发送功率错误导致的功率控制异常
        }
    }
    CAN_DISCONNECT_DETECTION_TIME_INDEX =
        0;  // 收到信息后重置CAN断连检测计数变量
}

void CAN_disconnect_detection() {}
