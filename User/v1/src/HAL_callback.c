#include "HAL_callback.h"

#include <math.h>

#include "MOS_driver.h"
#include "module_data.h"

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan,
                               uint32_t RxFifo0ITs) {
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
        CAN_receive();
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        Data_distribute(&adc_data);  // 处理ADC采样数据,包括分发数据和转换真实值
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    if (htim->Instance == TIM8) {
        uint32_t counter = __HAL_TIM_GET_COUNTER(&htim8);
        // 50kHz电流环PID计算和MOS驱动更新,过压保护
        if (PID_FREQUENCY_INDEX == counter % 2) {
            ADC_Transformer_voltage(&adc_data);
            ADC_Transformer_current(&adc_data);
            if (adc_data.V_CAP_TF > 27.0f) {
                MosDriver_stop(&mos_driver);
                PID_init(&power_pid_configs);
                PID_init(&current_pid_configs);
            }  // 过压保护,直接关闭占空比输出
            PID_calculate(&current_pid_configs,
                          current_pid_configs.target_value, adc_data.I_CAP_TF);
            MosDriver_dutylimit(&mos_driver, current_pid_configs.output);
        }
        // 20kHz功率环PID计算,电流环目标值更新,低压保护
        if (PID_FREQUENCY_INDEX == counter % 5) {
            chassis_power =
                adc_data.I_CHASSIS_TF * adc_data.V_CHASSIS_TF + 2.0f;
            if (adc_data.V_CHASSIS_TF > 19.0f) {
                dynamic_max_duty =
                    fminf(V_CAP_FULL / (adc_data.V_CHASSIS_TF + V_CAP_FULL),
                          MAX_DUTY);
                // 一阶低通：y += alpha * (x - y)
                dynamic_max_duty_pre +=
                    ALPHA * (dynamic_max_duty - dynamic_max_duty_pre);
                dynamic_max_duty = dynamic_max_duty_pre;
                current_pid_configs.OUT_MAX = dynamic_max_duty;
                mos_driver.OUT_MAX = dynamic_max_duty;
            } else {
                dynamic_max_duty = MAX_DUTY;
                dynamic_max_duty_pre = MAX_DUTY;
            }
            // 低压保护：低于阈值且非强充电状态，抬高最低占空比阻止继续放电
            if (adc_data.V_CAP_TF <= V_CAP_LOW_THRESHOLD &&
                adc_data.I_CAP_TF >= I_CAP_DISCHARGE_THRESHOLD) {
                mos_driver.OUT_MIN =
                    V_CAP_PROTECT_TARGET /
                    (adc_data.V_CHASSIS_TF + V_CAP_PROTECT_TARGET);
            } else {
                mos_driver.OUT_MIN = MIN_DUTY;
            }
            // 计算当前底盘功率，2W为系统损耗补偿
            PID_calculate(&power_pid_configs, power_pid_configs.target_value,
                          chassis_power);
            current_pid_configs.target_value = power_pid_configs.output;
        }
    }
    // 1kHz CAN发送和断连检测，断电检测
    if (htim->Instance == TIM16) {
        CAN_send();
        CAN_disconnect_detection();
        if (adc_data.I_CAP_ADC < -0.3f && adc_data.I_CHASSIS_ADC < 0.3f) {
            POWER_ERROR_DETECTION_TIME_INDEX++;
        }
        // 检测电池掉电情况，低于15V进行掉电检测自增
        else if (adc_data.V_CHASSIS_TF < 15.0f) {
            POWER_ERROR_DETECTION_TIME_INDEX += 10;
        } else {
            POWER_ERROR_DETECTION_TIME_INDEX = 0;
        }
        // 对于非电池断电情况进行掉电系数清零
        if (POWER_ERROR_DETECTION_TIME_INDEX >=
            MAX_POWER_ERROR_DETECTION_TIME) {
            MosDriver_stop(&mos_driver);
            PID_init(&power_pid_configs);
            PID_init(&current_pid_configs);
            POWER_ERROR_DETECTION_TIME_INDEX = MAX_POWER_ERROR_DETECTION_TIME;
            // 掉电检测计数变量达到最大值，执行掉电保护，停止MOS驱动，重置PID，防止计数变量溢出,并且只执行一次，直到系统重启
        }
    }
}
/*
控制环路运行流程图
1 所有模块初始化
2 CAN_receive接收目标功率和使能信号
3 调用一次电流环PID根据采样直接设置输出，初始化输出
4 触发HRTIM PWM输出
5 HRTIM Compare4事件触发ADC采样
6 ADC转换完成中断回调处理采样数据
7 定时器6中断回调进行数据校准、电流环PID计算和MOS驱动更新
8 定时器8中断回调进行功率环PID和电流环PID计算，过压检测，过流检测
9 定时器16中断回调进行CAN发送和通信断连检测，断电检测重复步骤4-9
*/