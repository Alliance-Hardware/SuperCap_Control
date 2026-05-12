#include "PID_controller.h"

void PID_init(PID_Configs* pid_configs) {
    pid_configs->error = 0;
    pid_configs->pre_error = 0;
    pid_configs->pre_pre_error = 0;
    pid_configs->output = 0;
    pid_configs->target_value = P_CHASSIS_MIN;
    // 为了防止掉电再上电后,CAN还没来得及发送当前需求功率，PID会先以最小功率要求运行
}

void PID_set(PID_Configs* pid_configs, float Kp, float Ki, float Kd,
             float OUT_MAX, float OUT_MIN, bool SWITCH) {
    pid_configs->SWITCH = SWITCH;
    pid_configs->Kp = Kp;
    pid_configs->Ki = Ki;
    pid_configs->Kd = Kd;
    pid_configs->OUT_MAX = OUT_MAX;
    pid_configs->OUT_MIN = OUT_MIN;
    pid_configs->target_value = P_CHASSIS_MIN;
}

void PID_calculate(PID_Configs* pid_configs, float target_point,
                   float real_value) {
    if (!pid_configs->SWITCH) {
        pid_configs->output = 0;
        pid_configs->error = 0;
        pid_configs->pre_error = 0;
        return;
    }
    pid_configs->error = target_point - real_value;
    pid_configs->output +=
        pid_configs->Kp * (pid_configs->error - pid_configs->pre_error) +
        pid_configs->Ki * pid_configs->error;
    if (pid_configs->output > pid_configs->OUT_MAX) {
        pid_configs->output = pid_configs->OUT_MAX;
    } else if (pid_configs->output < pid_configs->OUT_MIN) {
        pid_configs->output = pid_configs->OUT_MIN;
    }

    pid_configs->pre_error = pid_configs->error;
}
