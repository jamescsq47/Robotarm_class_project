#include "servo.h"
#include "tim.h"
#include <stddef.h>
static uint16_t current_pulse[SERVO_COUNT];
static int16_t current_pose[SERVO_COUNT];
static uint8_t initialized;
static uint8_t moving;
ServoStatus Servo_Init(void)
{
  if (moving) return SERVO_BUSY;
  if (initialized) return SERVO_OK;
  uint16_t pulse[SERVO_COUNT];
  if (!Servo_PoseToPulse(servo_zero_pose,pulse)) return SERVO_INVALID_ARGUMENT;
  MX_TIM2_Init();
  Servo_PWM_SetPulses(pulse);
  Servo_PWM_Start();
  HAL_Delay(SERVO_RAMP_STEP_MS);
  for (unsigned i=0;i<SERVO_COUNT;++i) {
    current_pulse[i]=pulse[i];
    current_pose[i]=servo_zero_pose[i];
  }
  initialized=1U;
  return SERVO_OK;
}
ServoStatus Servo_GetCommandedPose(int16_t out[SERVO_COUNT])
{
  if (!initialized) return SERVO_NOT_INITIALIZED;
  if (out==NULL) return SERVO_INVALID_ARGUMENT;
  if (moving) return SERVO_BUSY;
  for (unsigned i=0;i<SERVO_COUNT;++i) out[i]=current_pose[i];
  return SERVO_OK;
}
ServoStatus Servo_MoveTo(const int16_t pose[SERVO_COUNT],uint32_t duration_ms)
{
  uint16_t target[SERVO_COUNT],pulse[SERVO_COUNT];
  int16_t target_pose[SERVO_COUNT];
  if (!initialized) return SERVO_NOT_INITIALIZED;
  if (moving) return SERVO_BUSY;
  if (!Servo_PoseToPulse(pose,target)) return SERVO_INVALID_ARGUMENT;
  if (duration_ms==0U || duration_ms>SERVO_MAX_MOVE_MS ||
      duration_ms%SERVO_RAMP_STEP_MS!=0U) return SERVO_INVALID_DURATION;
  for (unsigned i=0;i<SERVO_COUNT;++i) target_pose[i]=pose[i];
  moving=1U;
  uint32_t steps=duration_ms/SERVO_RAMP_STEP_MS;
  uint32_t tick=HAL_GetTick();
  for (uint32_t step=1;step<=steps;++step) {
    tick+=SERVO_RAMP_STEP_MS;
    while ((int32_t)(HAL_GetTick()-tick)<0) __WFI();
    for (unsigned i=0;i<SERVO_COUNT;++i) {
      int32_t delta=(int32_t)target[i]-(int32_t)current_pulse[i];
      pulse[i]=(uint16_t)((int32_t)current_pulse[i]+delta*(int32_t)step/(int32_t)steps);
    }
    Servo_PWM_SetPulses(pulse);
  }
  /* Wait for the last buffered target to reach a complete PWM frame. */
  HAL_Delay(2U*SERVO_RAMP_STEP_MS);
  for (unsigned i=0;i<SERVO_COUNT;++i) {
    current_pulse[i]=target[i];
    current_pose[i]=target_pose[i];
  }
  moving=0U;
  return SERVO_OK;
}
