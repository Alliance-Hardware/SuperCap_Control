#pragma once
#include "ADC_Calibration.h"
#include "CAN_communicate.h"
#include "Data_collect.h"
#include "MOS_driver.h"
#include "PID_controller.h"
#include "const_data.h"
#include "module_data.h"
#include "stm32g474xx.h"
#include "stm32g4xx_hal_fdcan.h"
#include "stm32g4xx_hal_tim.h"
#include "tim.h"
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan,
                               uint32_t RxFifo0ITs);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);
