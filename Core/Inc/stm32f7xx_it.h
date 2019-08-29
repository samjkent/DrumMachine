/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void SysTick_Handler(void);
void TIM1_UP_TIM10_IRQHandler(void);
void OTG_HS_IRQHandler(void);

void DMA2_Stream6_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);

void ADC_IRQHandler(void);

void EXTI0_IRQHandler(void);
void EXTI9_5_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
