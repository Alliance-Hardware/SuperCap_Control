#include "ADC_Calibration.h"

bool ADC_ID_init() {  // RMCS_ID/LEGGED_ID 与 SUPERCAP_ID 的匹配检测
    uint32_t uid[3];
    uid[0] = HAL_GetUIDw0();
    uid[1] = HAL_GetUIDw1();
    uid[2] = HAL_GetUIDw2();
    for (int i = 0; i < 4; i++) {
        if (uid[0] == SUPERCAP_ID[i][0] && uid[1] == SUPERCAP_ID[i][1] &&
            uid[2] == SUPERCAP_ID[i][2]) {  // 匹配成功，分发对应
            for (int j = 0; j < 4; j++) {
                ADC_CALIBRATION_CONFIGS_BOARD[j][0] =
                    ADC_CALIBRATION_CONFIGS[i][j][0];
                ADC_CALIBRATION_CONFIGS_BOARD[j][1] =
                    ADC_CALIBRATION_CONFIGS[i][j][1];
            }
            // ADC_CALIBRATION_CONFIGS这个到时候改成转换函数的参数
            return true;
        }
    }
    return false;
}

void ADC_Transformer_voltage(datacollect* data) {
    // 分压比 11:1, ADC 分辨率 12bit, 参考电压 3.3V
    //  data->V_CHASSIS_TF = (float)data->V_CHASSIS_ADC / 4096.0f * 3.3f
    //  * 11.0f;
    data->V_CHASSIS_TF = (float)data->V_CHASSIS_ADC * 33.5f / 4096.0f;
    data->V_CAP_TF = (float)data->V_CAP_ADC * 33.5f / 4096.0f;
}

void ADC_Transformer_current(datacollect* data) {
    // INA240A2 基准电压 3.3V/2, 增益 50, ADC 分辨率 12bit, 采样电阻 0.002 欧姆
    // data->I_CHASSIS_TF = (float)(data->I_CHASSIS_ADC /
    // 4096.0f* 3.25f- 1.65f)/0.1f
    //  data->I_CAP_TF = (float)(data->I_CAP_ADC / 4096.0f* 3.25f - 1.65f) /0.1f

    data->I_CAP_TF = (float)(data->I_CAP_ADC * 0.0079345f - 16.2f);
    data->I_CHASSIS_TF = (float)(data->I_CHASSIS_ADC * 0.0079345f - 16.2f);
}
