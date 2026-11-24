#pragma once
#include "servo_motion.h"
typedef enum {
  SERVO_OK=0,
  SERVO_NOT_INITIALIZED,
  SERVO_INVALID_ARGUMENT,
  SERVO_INVALID_DURATION,
  SERVO_BUSY,
  SERVO_START_POSE_MISMATCH
} ServoStatus;
/* Initialize timer/GPIO and command zero once. Repeated calls do not move. */
ServoStatus Servo_Init(void);
/* Blocking, synchronized movement from the last completed commanded pose.
 * duration_ms: 20..600000, multiple of 20. No encoder feedback is read.
 */
ServoStatus Servo_MoveTo(const int16_t pose[SERVO_COUNT], uint32_t duration_ms);
/* Snapshot of last completed commanded pose; returns BUSY during a move. */
ServoStatus Servo_GetCommandedPose(int16_t out[SERVO_COUNT]);
