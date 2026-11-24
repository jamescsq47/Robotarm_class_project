#pragma once
#include "servo.h"
/* Blocking APIs; call serially from the main loop/task, never from an ISR. */
ServoStatus RobotArm_Init(void);
/* Action name: pick_above. Requires the completed commanded pose to be zero.
 * Ends at [180,60,0,0,0,0]. Does not silently reset or loop.
 */
ServoStatus RobotArm_PickAbove(void);
/* Reset from the recorded completed target to zero in 5000 ms. */
ServoStatus RobotArm_Reset(void);
/* Wrapper also excludes queries during the action's dwell intervals. */
ServoStatus RobotArm_GetCommandedPose(int16_t out[SERVO_COUNT]);
