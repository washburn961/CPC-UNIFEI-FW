/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PROCESSING_TIMING_Pin GPIO_PIN_4
#define PROCESSING_TIMING_GPIO_Port GPIOE
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define USER_LED0_Pin GPIO_PIN_0
#define USER_LED0_GPIO_Port GPIOB
#define USER_LED1_Pin GPIO_PIN_1
#define USER_LED1_GPIO_Port GPIOB
#define USER_LED2_Pin GPIO_PIN_2
#define USER_LED2_GPIO_Port GPIOB
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOB
#define RMII_TXD0_Pin GPIO_PIN_12
#define RMII_TXD0_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define ADC_BUSY_Pin GPIO_PIN_11
#define ADC_BUSY_GPIO_Port GPIOD
#define ADC_BUSY_EXTI_IRQn EXTI15_10_IRQn
#define ADC_CONVST_Pin GPIO_PIN_12
#define ADC_CONVST_GPIO_Port GPIOD
#define ADC_RESET_Pin GPIO_PIN_13
#define ADC_RESET_GPIO_Port GPIOD
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define OUT1_A_OUT_Pin GPIO_PIN_0
#define OUT1_A_OUT_GPIO_Port GPIOD
#define OUT4_A_OUT_Pin GPIO_PIN_1
#define OUT4_A_OUT_GPIO_Port GPIOD
#define OUT3_A_OUT_Pin GPIO_PIN_3
#define OUT3_A_OUT_GPIO_Port GPIOD
#define OUT2_A_OUT_Pin GPIO_PIN_4
#define OUT2_A_OUT_GPIO_Port GPIOD
#define OUT1_B_OUT_Pin GPIO_PIN_5
#define OUT1_B_OUT_GPIO_Port GPIOD
#define OUT3_B_OUT_Pin GPIO_PIN_6
#define OUT3_B_OUT_GPIO_Port GPIOD
#define OUT2_B_OUT_Pin GPIO_PIN_9
#define OUT2_B_OUT_GPIO_Port GPIOG
#define OUT4_B_OUT_Pin GPIO_PIN_10
#define OUT4_B_OUT_GPIO_Port GPIOG
#define ANALOG_TIMING_Pin GPIO_PIN_8
#define ANALOG_TIMING_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
