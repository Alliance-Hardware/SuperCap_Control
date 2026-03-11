#include "Beef.h"

void Beef_init() {
    __HAL_TIM_SET_AUTORELOAD(&htim2, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void Beef_set(int frequency) {
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    if (frequency <= 0) {
        __HAL_TIM_SET_AUTORELOAD(&htim2, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        return;
    }
    int autoreload = 1000000 / frequency - 1;
    int pulse = autoreload * 0.5;
    __HAL_TIM_SET_AUTORELOAD(&htim2, autoreload);
    // 设置自动重装载寄存器周期的值，以此来改变PWM周期的时常，即频率
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
    // 设置捕获比较寄存器的值，以此来改变PWM的占空比，这里设置为50%，即标准方波
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_Delay(100);
}

void Beef_play() {
    uint16_t freq_seq[] = {293, 329, 349, 392, 440, 493, 261};
    for (uint8_t index = 0; index < sizeof(freq_seq) / sizeof(freq_seq[0]);
         index++) {
        Beef_set(freq_seq[index]);
    }
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}