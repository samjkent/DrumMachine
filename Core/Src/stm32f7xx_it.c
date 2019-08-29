/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "stm32f7xx_it.h"
#include "cmsis_os.h"
#include "main.h"

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_tim8_ch2;
extern TIM_HandleTypeDef htim10;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

extern TIM_HandleTypeDef htim1;

extern SAI_HandleTypeDef SaiHandle;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

/******************************************************************************/
/*            Cortex-M7 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  osSystickHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
*/
void TIM1_UP_TIM10_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  HAL_TIM_IRQHandler(&htim10);
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
* @brief This function handles USB On The Go HS global interrupt.
*/
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(SaiHandle.hdmatx);
}

void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_tim8_ch2);
}

/**
* @brief This function handles interrupt for EXTI0
*/
void EXTI0_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


/**
* @brief This function handles interrupt for EXTI9_5
*/
void EXTI9_5_IRQHandler(void) {
    NVIC_DisableIRQ(EXTI9_5_IRQn);

    if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_6) != RESET ) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
        HAL_GPIO_EXTI_Callback(GPIO_PIN_6);
    }
    else if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_7) != RESET ) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    }
    else if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_8) != RESET ) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
        HAL_GPIO_EXTI_Callback(GPIO_PIN_8);
    }
    else if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_9) != RESET ) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    }
        
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    
}

void DMA2_Stream0_IRQHandler(void)
{
   HAL_DMA_IRQHandler(&hdma_adc1);
}

void ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&hadc1);
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
