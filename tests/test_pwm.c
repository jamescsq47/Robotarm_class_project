#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
GPIO_TypeDef mock_a,mock_b,mock_c;
TIM_TypeDef mock_tim;
void Error_Handler(void){abort();}
#include "../Core/Src/tim.c"
int main(void)
{
  uint16_t width[6],later[6]={1000,1000,1000,1000,1000,1000};
  MX_TIM2_Init();
  assert(htim2.Init.Period==19999 && htim2.Init.Prescaler==15);
  assert(ports[0]==GPIOA && pins[0]==2);
  assert(ports[1]==GPIOC && pins[1]==8);
  assert(ports[2]==GPIOA && pins[2]==8);
  assert(ports[3]==GPIOA && pins[3]==32);
  assert(ports[4]==GPIOC && pins[4]==16);
  assert(ports[5]==GPIOB && pins[5]==1);
  for(unsigned frame=0;frame<10000;frame++){
    for(unsigned i=0;i<6;i++) width[i]=(uint16_t)(500+rand()%2001);
    if(frame==0) for(unsigned i=0;i<6;i++) width[i]=1500;
    if(frame==1) for(unsigned i=0;i<6;i++) width[i]=(uint16_t)(500+i);
    Servo_PWM_SetPulses(width);
    mock_tim.CNT=5; mock_tim.SR=TIM_SR_UIF; TIM2_IRQHandler();
    assert(active==63);
    for(unsigned i=0;i<6;i++) assert(deadline[i]==5U+width[i]);
    Servo_PWM_SetPulses(later); /* next frame only */
    unsigned calls=0;
    while(active){
      uint32_t target=mock_tim.CCR1;
      mock_tim.CNT=target+(frame%2 ? 100U : 0U); /* delayed ISR */
      mock_tim.SR=TIM_SR_CC1IF; TIM2_IRQHandler();
      for(unsigned i=0;i<6;i++)
        assert(!!(active&(1U<<i))==(deadline[i]>mock_tim.CNT));
      assert(++calls<=6);
    }
    assert(mock_tim.DIER==TIM_DIER_UIE);
  }
  puts("PASS: 10000 PWM frames, pin map, equal/close edges, late IRQ, frame buffering");
}
