#pragma once
#include "main.h"
#include "servo_motion.h"
extern TIM_HandleTypeDef htim2;
void MX_TIM2_Init(void);
void Servo_PWM_SetPulses(const uint16_t pulse[SERVO_COUNT]);
void Servo_PWM_Start(void);
void TIM2_IRQHandler(void);
