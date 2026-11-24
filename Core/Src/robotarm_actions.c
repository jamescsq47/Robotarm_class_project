#include "robotarm_actions.h"
#include "main.h"
static uint8_t action_busy;
ServoStatus RobotArm_Init(void)
{
  if (action_busy) return SERVO_BUSY;
  return Servo_Init();
}
ServoStatus RobotArm_GetCommandedPose(int16_t out[SERVO_COUNT])
{
  if (action_busy) return SERVO_BUSY;
  return Servo_GetCommandedPose(out);
}
ServoStatus RobotArm_PickAbove(void)
{
  int16_t current[SERVO_COUNT];
  uint16_t checked[SERVO_COUNT];
  if (action_busy) return SERVO_BUSY;
  ServoStatus status=Servo_GetCommandedPose(current);
  if (status!=SERVO_OK) return status;
  for (unsigned i=0;i<SERVO_COUNT;++i)
    if (current[i]!=servo_zero_pose[i]) return SERVO_START_POSE_MISMATCH;
  /* Preflight the entire action so an invalid later stage cannot start it. */
  for (unsigned s=0;s<PICK_ABOVE_STAGE_COUNT;++s) {
    const ArmStage *stage=&pick_above_stages[s];
    if (!Servo_PoseToPulse(stage->pose,checked)) return SERVO_INVALID_ARGUMENT;
    if (stage->move_ms==0U || stage->move_ms>SERVO_MAX_MOVE_MS ||
        stage->move_ms%SERVO_RAMP_STEP_MS!=0U) return SERVO_INVALID_DURATION;
  }
  action_busy=1U;
  for (unsigned s=0;s<PICK_ABOVE_STAGE_COUNT;++s) {
    status=Servo_MoveTo(pick_above_stages[s].pose,pick_above_stages[s].move_ms);
    if (status!=SERVO_OK) break;
    if (pick_above_stages[s].hold_ms!=0U) HAL_Delay(pick_above_stages[s].hold_ms);
  }
  action_busy=0U;
  return status;
}
ServoStatus RobotArm_Reset(void)
{
  if (action_busy) return SERVO_BUSY;
  action_busy=1U;
  ServoStatus status=Servo_MoveTo(servo_zero_pose,5000U);
  action_busy=0U;
  return status;
}
