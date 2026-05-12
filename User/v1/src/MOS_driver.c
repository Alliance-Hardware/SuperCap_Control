#include "MOS_driver.h"

#include "const_data.h"

void MosDriver_TIMER_init() {
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_B);
}

void MosDriver_OUTPUT_init() {
    HAL_HRTIM_WaveformOutputStart(
        &hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 |
                      HRTIM_OUTPUT_TB2);
}

void MosDriver_chassis_set(float chassis_duty, mosdriver* driver) {
    driver->chassis_compare1_index =
        HALF_CYCLE_INDEX - chassis_duty * HALF_CYCLE_INDEX * DUTY_INDEX;
    driver->chassis_compare3_index =
        HALF_CYCLE_INDEX + chassis_duty * HALF_CYCLE_INDEX * DUTY_INDEX;
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A,
                           HRTIM_COMPAREUNIT_1, driver->chassis_compare1_index);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A,
                           HRTIM_COMPAREUNIT_3, driver->chassis_compare3_index);
}

void MosDriver_cap_set(float cap_duty, mosdriver* driver) {
    driver->cap_compare1_index =
        CYCLE_INDEX - cap_duty * HALF_CYCLE_INDEX * DUTY_INDEX;
    driver->cap_compare3_index =
        CYCLE_ZERO + cap_duty * HALF_CYCLE_INDEX * DUTY_INDEX;
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B,
                           HRTIM_COMPAREUNIT_1, driver->cap_compare1_index);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B,
                           HRTIM_COMPAREUNIT_3, driver->cap_compare3_index);
}

void MosDriver_init(mosdriver* driver) {
    driver->chassis_compare1_index = 0.0;
    driver->chassis_compare3_index = 0.0;
    driver->cap_compare1_index = 0.0;
    driver->cap_compare3_index = 0.0;
    driver->Phase_shift_angle = 0.0;
    driver->OUT_MAX = MAX_DUTY;
    driver->OUT_MIN = MIN_DUTY;
}

void MosDriver_stop(mosdriver* driver) {
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
                                               HRTIM_OUTPUT_TB1 |
                                               HRTIM_OUTPUT_TB2);
    driver->chassis_compare1_index = 0;
    driver->chassis_compare3_index = 0;
    driver->cap_compare1_index = 0;
    driver->cap_compare3_index = 0;
    driver->Phase_shift_angle = 0;
}

void MosDriver_dutylimit(mosdriver* driver, float duty) {
    if (duty > driver->OUT_MAX) {
        duty = driver->OUT_MAX;
    } else if (duty < driver->OUT_MIN) {
        duty = driver->OUT_MIN;
    }
    MosDriver_chassis_set(duty, driver);
    MosDriver_cap_set(1 - duty, driver);
}