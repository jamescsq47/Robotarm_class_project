#include "servo_motion.h"
#include <stddef.h>
/* Absolute 0..180 servo mapping: 500..2500 us. No zero offsets applied. */
const int16_t servo_zero_pose[SERVO_COUNT]={0,0,0,0,0,0};
const ArmStage pick_above_stages[PICK_ABOVE_STAGE_COUNT]={
  {{0,135,10,35,0,0},5000U,1000U},
  {{0,135,10,35,0,90},2000U,0U},
  {{0,90,90,35,0,90},3000U,0U},
  {{180,90,90,35,0,90},2000U,0U},
  {{180,60,0,0,0,90},3000U,0U},
  {{180,60,0,0,0,0},1000U,0U}
};
int Servo_PoseToPulse(const int16_t pose[SERVO_COUNT], uint16_t pulse[SERVO_COUNT])
{
  if (pose==NULL || pulse==NULL) return 0;
  for (unsigned i=0;i<SERVO_COUNT;++i)
    if (pose[i]<0 || pose[i]>180) return 0;
  for (unsigned i=0;i<SERVO_COUNT;++i)
    pulse[i]=(uint16_t)(500U+((uint32_t)pose[i]*2000U+90U)/180U);
  return 1;
}
