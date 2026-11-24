#pragma once
#include <stdint.h>
typedef struct { volatile uint32_t BSRR; } GPIO_TypeDef;
typedef struct { volatile uint32_t CR1,DIER,EGR,SR,CNT,CCR1; } TIM_TypeDef;
extern GPIO_TypeDef mock_a,mock_b,mock_c;
extern TIM_TypeDef mock_tim;
#define GPIOA (&mock_a)
#define GPIOB (&mock_b)
#define GPIOC (&mock_c)
#define TIM2 (&mock_tim)
#define GPIO_PIN_0 1U
#define GPIO_PIN_1 2U
#define GPIO_PIN_3 8U
#define GPIO_PIN_4 16U
#define GPIO_PIN_5 32U
#define GPIO_MODE_OUTPUT_PP 1U
#define GPIO_NOPULL 0U
#define GPIO_SPEED_FREQ_HIGH 2U
#define TIM_COUNTERMODE_UP 0U
#define TIM_CLOCKDIVISION_DIV1 0U
#define TIM_AUTORELOAD_PRELOAD_DISABLE 0U
#define TIM2_IRQn 28
#define TIM_CR1_CEN 1U
#define TIM_EGR_UG 1U
#define TIM_SR_UIF 1U
#define TIM_SR_CC1IF 2U
#define TIM_DIER_UIE 1U
#define TIM_DIER_CC1IE 2U
#define HAL_OK 0
#define __HAL_RCC_TIM2_CLK_ENABLE() ((void)0)
#define __HAL_RCC_GPIOA_CLK_ENABLE() ((void)0)
#define __HAL_RCC_GPIOB_CLK_ENABLE() ((void)0)
#define __HAL_RCC_GPIOC_CLK_ENABLE() ((void)0)
typedef struct {uint32_t Pin,Mode,Pull,Speed;} GPIO_InitTypeDef;
typedef struct {TIM_TypeDef *Instance; struct {uint32_t Prescaler,CounterMode,Period,ClockDivision,AutoReloadPreload;} Init;} TIM_HandleTypeDef;
static inline void HAL_GPIO_Init(GPIO_TypeDef *p,GPIO_InitTypeDef *g){(void)p;(void)g;}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *);
static inline int HAL_TIM_Base_Init(TIM_HandleTypeDef *h){HAL_TIM_Base_MspInit(h);return HAL_OK;}
static inline void HAL_NVIC_SetPriority(int i,int a,int b){(void)i;(void)a;(void)b;}
static inline void HAL_NVIC_EnableIRQ(int i){(void)i;}
static inline uint32_t __get_PRIMASK(void){return 0;}
static inline void __disable_irq(void){}
static inline void __set_PRIMASK(uint32_t p){(void)p;}
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t ms);
void mock_wait(void);
#define __WFI() mock_wait()
