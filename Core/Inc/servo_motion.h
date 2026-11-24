#pragma once
#include <stdint.h>
#define SERVO_COUNT 6U
#define SERVO_FRAME_US 20000U
#define SERVO_RAMP_STEP_MS 20U
#define SERVO_MAX_MOVE_MS 600000U
#define PICK_ABOVE_STAGE_COUNT 6U
typedef struct {
  int16_t pose[SERVO_COUNT];
  uint32_t move_ms;
  uint32_t hold_ms;
} ArmStage;
extern const int16_t servo_zero_pose[SERVO_COUNT];
extern const ArmStage pick_above_stages[PICK_ABOVE_STAGE_COUNT];
int Servo_PoseToPulse(const int16_t pose[SERVO_COUNT], uint16_t pulse[SERVO_COUNT]);
