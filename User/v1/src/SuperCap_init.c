#include "SuperCap_init.h"

#include "CAN_communicate.h"
#include "Data_collect.h"
#include "MOS_driver.h"
#include "PID_controller.h"
#include "module_data.h"

void SuperCap_init(void) {
    if (ADC_ID_init()) {
        CAN_init();
        MosDriver_init(&mos_driver);
        MosDriver_TIMER_init();
        Data_init(&adc_data);
        PID_init(&current_pid_configs);
        PID_init(&voltage_pid_configs);
        PID_init(&power_pid_configs);
        PID_set(&current_pid_configs, 0.002f, 0.00002f, 0.0f, MAX_DUTY,
                MIN_DUTY, true);
        // 电流环PID参数设置
        PID_set(&voltage_pid_configs, 0.0f, 0.0f, 0.0f, V_CAP_MAX, V_CAP_MIN,
                false);
        // 电压环PID参数设置
        PID_set(&power_pid_configs, 0.04f, 0.006f, 0.0f, I_CAP_MAX, I_CAP_MIN,
                true);
        // 功率环PID参数设置
    } else {
        Error_Handler();
    }
}

void SuperCap_softstart() {
    HAL_Delay(10);
    // 等待数据获取，ADC采样-转换完成
    Data_distribute(&adc_data);
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
    // 直接设置电流环输出为软启动占空比，PID的初始占空比设置，可以理解为前馈
    HAL_Delay(10);
    // 等待软启动完成
}
