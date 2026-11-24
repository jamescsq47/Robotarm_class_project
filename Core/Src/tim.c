#include "tim.h"
TIM_HandleTypeDef htim2;
/* P1..P6: PA1, PC3, PA3, PA5, PC4, PB0.
 * PC3/PC4 do not expose hardware timer PWM. TIM2 schedules all six GPIO
 * pulse edges (at most 350 interrupts/s). Tick=1 us; frame=20 ms.
 */
static GPIO_TypeDef *const ports[SERVO_COUNT]={GPIOA,GPIOC,GPIOA,GPIOA,GPIOC,GPIOB};
static const uint16_t pins[SERVO_COUNT]={GPIO_PIN_1,GPIO_PIN_3,GPIO_PIN_3,GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_0};
static volatile uint16_t pending[SERVO_COUNT]={1500,1500,1500,1500,1500,1500};
static uint32_t deadline[SERVO_COUNT];
static uint32_t active;
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *h)
{
  if (h->Instance==TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
}
void MX_TIM2_Init(void)
{
  GPIO_InitTypeDef gpio={0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOA->BSRR=(GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_5)<<16U;
  GPIOC->BSRR=(GPIO_PIN_3|GPIO_PIN_4)<<16U;
  GPIOB->BSRR=GPIO_PIN_0<<16U;
  gpio.Mode=GPIO_MODE_OUTPUT_PP;
  gpio.Pull=GPIO_NOPULL;
  gpio.Speed=GPIO_SPEED_FREQ_HIGH;
  gpio.Pin=GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_5; HAL_GPIO_Init(GPIOA,&gpio);
  gpio.Pin=GPIO_PIN_3|GPIO_PIN_4; HAL_GPIO_Init(GPIOC,&gpio);
  gpio.Pin=GPIO_PIN_0; HAL_GPIO_Init(GPIOB,&gpio);
  htim2.Instance=TIM2;
  htim2.Init.Prescaler=15;
  htim2.Init.CounterMode=TIM_COUNTERMODE_UP;
  htim2.Init.Period=SERVO_FRAME_US-1U;
  htim2.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2)!=HAL_OK) Error_Handler();
  HAL_NVIC_SetPriority(TIM2_IRQn,1,0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
void Servo_PWM_SetPulses(const uint16_t pulse[SERVO_COUNT])
{
  for (unsigned i=0;i<SERVO_COUNT;++i)
    if (pulse[i]<500U || pulse[i]>2500U) Error_Handler();
  uint32_t mask=__get_PRIMASK();
  __disable_irq();
  for (unsigned i=0;i<SERVO_COUNT;++i) pending[i]=pulse[i];
  __set_PRIMASK(mask);
}
void Servo_PWM_Start(void)
{
  TIM2->CR1&=~TIM_CR1_CEN;
  TIM2->DIER=0;
  TIM2->EGR=TIM_EGR_UG;
  TIM2->SR=0;
  TIM2->CNT=SERVO_FRAME_US-1U;
  TIM2->DIER=TIM_DIER_UIE;
  TIM2->CR1|=TIM_CR1_CEN;
}
void TIM2_IRQHandler(void)
{
  if ((TIM2->SR&TIM_SR_UIF)!=0U) {
    uint16_t width[SERVO_COUNT];
    TIM2->SR=~(TIM_SR_UIF|TIM_SR_CC1IF);
    for (unsigned i=0;i<SERVO_COUNT;++i) width[i]=pending[i];
    GPIOA->BSRR=GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_5;
    GPIOC->BSRR=GPIO_PIN_3|GPIO_PIN_4;
    GPIOB->BSRR=GPIO_PIN_0;
    uint32_t anchor=TIM2->CNT;
    uint32_t first=SERVO_FRAME_US;
    for (unsigned i=0;i<SERVO_COUNT;++i) {
      deadline[i]=anchor+width[i];
      if (deadline[i]<first) first=deadline[i];
    }
    active=(1U<<SERVO_COUNT)-1U;
    TIM2->CCR1=first;
    TIM2->DIER=TIM_DIER_UIE|TIM_DIER_CC1IE;
    return;
  }
  if ((TIM2->SR&TIM_SR_CC1IF)!=0U && (TIM2->DIER&TIM_DIER_CC1IE)!=0U) {
    TIM2->SR=~TIM_SR_CC1IF;
    /* Recheck after writing CCR1 so close edges cannot miss the compare. */
    for (;;) {
      uint32_t now=TIM2->CNT;
      uint32_t next=SERVO_FRAME_US;
      for (unsigned i=0;i<SERVO_COUNT;++i) {
        if ((active&(1U<<i))==0U) continue;
        if (deadline[i]<=now) {
          ports[i]->BSRR=(uint32_t)pins[i]<<16U;
          active&=~(1U<<i);
        } else if (deadline[i]<next) next=deadline[i];
      }
      if (active==0U) { TIM2->DIER=TIM_DIER_UIE; return; }
      TIM2->CCR1=next;
      if (TIM2->CNT<next) return;
    }
  }
}
