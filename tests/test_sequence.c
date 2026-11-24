/* Exercises production action + servo layers; only board I/O/time are mocked. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "robotarm_actions.h"
typedef struct {uint32_t tick; uint16_t p[6];} Sample;
static Sample samples[4096];
static unsigned count,timer_inits,starts;
static uint32_t tick;
static unsigned probe_move,probe_dwell,probed_move,probed_dwell;
static const int16_t zero[6]={0,0,0,0,0,0};
static const int16_t poses[6][6]={
  {0,135,10,35,0,0},{0,135,10,35,0,90},{0,90,90,35,0,90},
  {180,90,90,35,0,90},{180,60,0,0,0,90},{180,60,0,0,0,0}
};
static const uint16_t ends[6][6]={
  {500,2000,611,889,500,500},{500,2000,611,889,500,1500},
  {500,1500,1500,889,500,1500},{2500,1500,1500,889,500,1500},
  {2500,1167,500,500,500,1500},{2500,1167,500,500,500,500}
};
static const uint32_t durations[6]={5000,2000,3000,2000,3000,1000};
static const unsigned boundaries[6]={250,350,500,600,750,800};
static const uint32_t end_times[6]={5000,8040,11080,13120,16160,17200};
void Error_Handler(void){abort();}
uint32_t HAL_GetTick(void){return tick;}
void mock_wait(void){++tick;}
static void check_busy(void)
{
  int16_t out[6];
  assert(RobotArm_PickAbove()==SERVO_BUSY);
  assert(RobotArm_Reset()==SERVO_BUSY);
  assert(RobotArm_Init()==SERVO_BUSY);
  assert(RobotArm_GetCommandedPose(out)==SERVO_BUSY);
}
void HAL_Delay(uint32_t ms)
{
  if (probe_dwell && ms==1000U) {
    probe_dwell=0; check_busy(); ++probed_dwell;
  }
  tick+=ms;
}
void MX_TIM2_Init(void){++timer_inits;}
void Servo_PWM_Start(void){++starts;}
void Servo_PWM_SetPulses(const uint16_t p[6])
{
  assert(count<4096);
  samples[count].tick=tick;
  for(unsigned i=0;i<6;i++){
    assert(p[i]>=500 && p[i]<=2500);
    samples[count].p[i]=p[i];
  }
  ++count;
  if(probe_move){probe_move=0;check_busy();++probed_move;}
}
static void check_pose(const int16_t expected[6])
{
  int16_t out[6];
  assert(RobotArm_GetCommandedPose(out)==SERVO_OK);
  assert(!memcmp(out,expected,sizeof(out)));
}
static void check_pick(unsigned offset,uint32_t began)
{
  unsigned stage=0;
  for(unsigned n=1;n<=800;n++){
    while(n>boundaries[stage]) ++stage;
    const Sample *s=&samples[offset+n-1];
    unsigned first=stage==0?1:boundaries[stage-1]+1;
    uint32_t previous=n==1?began:samples[offset+n-2].tick;
    uint32_t gap=n==first?(stage==0?20U:(stage==1?1060U:60U)):20U;
    assert((uint32_t)(s->tick-previous)==gap);
    for(unsigned i=0;i<6;i++){
      int begin=stage==0?500:ends[stage-1][i];
      int end=ends[stage][i];
      int prev=n==1?500:samples[offset+n-2].p[i];
      int steps=(int)(durations[stage]/20);
      int max_step=(abs(end-begin)+steps-1)/steps;
      assert(abs((int)s->p[i]-prev)<=max_step);
      if(end>=begin) assert(s->p[i]>=prev && s->p[i]<=end);
      else assert(s->p[i]<=prev && s->p[i]>=end);
    }
    if(stage>=2 && stage<=4) assert(s->p[5]==1500);
    if(n==boundaries[stage]){
      assert((uint32_t)(s->tick-began)==end_times[stage]);
      assert(!memcmp(s->p,ends[stage],sizeof(s->p)));
    }
  }
}
static void check_reset(void)
{
  unsigned offset=count;
  uint32_t began=tick;
  uint16_t from[6];
  memcpy(from,samples[count-1].p,sizeof(from));
  assert(RobotArm_Reset()==SERVO_OK);
  assert(count==offset+250);
  assert((uint32_t)(tick-began)==5040);
  for(unsigned n=1;n<=250;n++){
    Sample *s=&samples[offset+n-1];
    assert((uint32_t)(s->tick-began)==n*20U);
    for(unsigned i=0;i<6;i++){
      int previous=n==1?from[i]:samples[offset+n-2].p[i];
      int bound=((int)from[i]-500+249)/250;
      assert(s->p[i]<=previous && s->p[i]>=500);
      assert(previous-s->p[i]<=bound);
    }
  }
  for(unsigned i=0;i<6;i++) assert(samples[count-1].p[i]==500);
  check_pose(zero);
}
int main(void)
{
  int16_t out[6],bad[6]={0,0,0,-1,0,0};
  uint16_t pulse[6]={1,2,3,4,5,6},saved[6];
  memcpy(saved,pulse,sizeof(saved));
  assert(RobotArm_PickAbove()==SERVO_NOT_INITIALIZED);
  assert(RobotArm_Reset()==SERVO_NOT_INITIALIZED);
  assert(RobotArm_GetCommandedPose(out)==SERVO_NOT_INITIALIZED);
  assert(count==0 && tick==0);
  assert(!Servo_PoseToPulse(NULL,pulse));
  assert(!Servo_PoseToPulse(zero,NULL));
  assert(!Servo_PoseToPulse(bad,pulse));
  assert(!memcmp(pulse,saved,sizeof(pulse)));
  bad[3]=181;assert(!Servo_PoseToPulse(bad,pulse));
  assert(RobotArm_Init()==SERVO_OK);
  assert(count==1 && timer_inits==1 && starts==1 && tick==20);
  check_pose(zero);
  assert(RobotArm_GetCommandedPose(NULL)==SERVO_INVALID_ARGUMENT);
  assert(RobotArm_Init()==SERVO_OK);
  assert(count==1 && tick==20);
  assert(Servo_MoveTo(NULL,1000)==SERVO_INVALID_ARGUMENT);
  assert(Servo_MoveTo(bad,1000)==SERVO_INVALID_ARGUMENT);
  assert(Servo_MoveTo(zero,0)==SERVO_INVALID_DURATION);
  assert(Servo_MoveTo(zero,21)==SERVO_INVALID_DURATION);
  assert(Servo_MoveTo(zero,600020)==SERVO_INVALID_DURATION);
  assert(count==1 && tick==20);
  for(unsigned s=0;s<6;s++){
    assert(!memcmp(pick_above_stages[s].pose,poses[s],sizeof(poses[s])));
    assert(pick_above_stages[s].move_ms==durations[s]);
    assert(pick_above_stages[s].hold_ms==(s==0?1000U:0U));
  }
  unsigned offset=count;uint32_t began=tick;
  probe_move=probe_dwell=1;
  assert(RobotArm_PickAbove()==SERVO_OK);
  assert(probed_move==1 && probed_dwell==1);
  assert(count==offset+800 && (uint32_t)(tick-began)==17240);
  check_pick(offset,began);
  check_pose(poses[5]);
  unsigned saved_count=count;uint32_t saved_tick=tick;
  assert(RobotArm_Init()==SERVO_OK); /* Must not jump back to zero. */
  check_pose(poses[5]);
  assert(RobotArm_PickAbove()==SERVO_START_POSE_MISMATCH);
  assert(count==saved_count && tick==saved_tick);
  check_reset(); /* From [180,60,0,0,0,0], not a hard-coded start. */
  check_reset(); /* Calling reset at zero still holds a 5s trajectory. */
  const int16_t custom[6]={30,60,90,45,20,10};
  assert(Servo_MoveTo(custom,1000)==SERVO_OK);
  check_pose(custom);
  assert(RobotArm_PickAbove()==SERVO_START_POSE_MISMATCH);
  check_reset(); /* A different prior endpoint must also reset correctly. */
  tick=UINT32_MAX-1000U;began=tick;
  assert(Servo_MoveTo(custom,2000)==SERVO_OK);
  assert((uint32_t)(tick-began)==2040);
  check_reset();
  offset=count;began=tick;
  assert(RobotArm_PickAbove()==SERVO_OK); /* Reusable after reset. */
  assert(count==offset+800 && (uint32_t)(tick-began)==17240);
  check_pick(offset,began);
  check_pose(poses[5]);
  assert(timer_inits==1 && starts==1);
  puts("PASS: pick_above 180deg endpoints/timing; stateful 5s reset from multiple poses; init idempotence; preconditions/busy; bounds; tick wrap");
}
