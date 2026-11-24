/* USER CODE BEGIN Header */
/* Example: initialize, explicitly run pick_above once, then hold. */
/* USER CODE END Header */
#include "main.h"
#include "tim.h"
/* USER CODE BEGIN Includes */
#include "robotarm_actions.h"
/* USER CODE END Includes */
void SystemClock_Config(void);
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  /* USER CODE BEGIN 2 */
  if (RobotArm_Init()!=SERVO_OK) Error_Handler();
  if (RobotArm_PickAbove()!=SERVO_OK) Error_Handler();
  /* To return to zero, explicitly call RobotArm_Reset() here or later. */
  /* USER CODE END 2 */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    __WFI();
    /* USER CODE END 3 */
  }
}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0}; RCC_ClkInitTypeDef clk = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clk,FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE(); __HAL_RCC_PWR_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}
void SysTick_Handler(void) { HAL_IncTick(); }
void Error_Handler(void) { __disable_irq(); while(1) {} }

