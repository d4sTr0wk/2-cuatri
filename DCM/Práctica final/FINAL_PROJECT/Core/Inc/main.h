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
#include "stm32l4xx_hal.h"

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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define Display1_Pin GPIO_PIN_0
#define Display1_GPIO_Port GPIOC
#define Display2_Pin GPIO_PIN_1
#define Display2_GPIO_Port GPIOC
#define Fil2_Pin GPIO_PIN_2
#define Fil2_GPIO_Port GPIOC
#define Fil1_Pin GPIO_PIN_3
#define Fil1_GPIO_Port GPIOC
#define CLK_Pin GPIO_PIN_1
#define CLK_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define Display4_Pin GPIO_PIN_4
#define Display4_GPIO_Port GPIOA
#define Col4_Pin GPIO_PIN_5
#define Col4_GPIO_Port GPIOA
#define Col3_Pin GPIO_PIN_6
#define Col3_GPIO_Port GPIOA
#define Col2_Pin GPIO_PIN_7
#define Col2_GPIO_Port GPIOA
#define Button3_Pin GPIO_PIN_5
#define Button3_GPIO_Port GPIOC
#define Display3_Pin GPIO_PIN_0
#define Display3_GPIO_Port GPIOB
#define Point_Pin GPIO_PIN_1
#define Point_GPIO_Port GPIOB
#define Seg2_Pin GPIO_PIN_10
#define Seg2_GPIO_Port GPIOB
#define Seg4_Pin GPIO_PIN_13
#define Seg4_GPIO_Port GPIOB
#define Seg5_Pin GPIO_PIN_14
#define Seg5_GPIO_Port GPIOB
#define Seg6_Pin GPIO_PIN_15
#define Seg6_GPIO_Port GPIOB
#define Button4_Pin GPIO_PIN_6
#define Button4_GPIO_Port GPIOC
#define Led4_Pin GPIO_PIN_9
#define Led4_GPIO_Port GPIOC
#define Seg3_Pin GPIO_PIN_8
#define Seg3_GPIO_Port GPIOA
#define Button1_Pin GPIO_PIN_11
#define Button1_GPIO_Port GPIOA
#define Button2_Pin GPIO_PIN_12
#define Button2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define Fil4_Pin GPIO_PIN_10
#define Fil4_GPIO_Port GPIOC
#define Fil3_Pin GPIO_PIN_12
#define Fil3_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define Seg1_Pin GPIO_PIN_4
#define Seg1_GPIO_Port GPIOB
#define Seg0_Pin GPIO_PIN_5
#define Seg0_GPIO_Port GPIOB
#define Col1_Pin GPIO_PIN_6
#define Col1_GPIO_Port GPIOB
#define Led3_Pin GPIO_PIN_8
#define Led3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
