/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define INT_MAX 4294967295
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

const int matriz_teclado[4][4] = { { 0xC, 0xD, 0xE, 0xF },
		{ 0xB, 0x9, 0x6, 0x3 }, { 0x0, 0x8, 0x5, 0x2 }, { 0xA, 0x7, 0x4, 0x1 } };

int lectura_columnas() {
	if (HAL_GPIO_ReadPin(GPIOB, Col1_Pin))
		return (1);
	else if (HAL_GPIO_ReadPin(GPIOA, Col2_Pin))
		return (2);
	else if (HAL_GPIO_ReadPin(GPIOA, Col3_Pin))
		return (3);
	else if (HAL_GPIO_ReadPin(GPIOA, Col4_Pin))
		return (4);

	return (0);
}

void write_num_display_dec(int digit, int point) {
	switch (digit) {
	case (0):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		break;
	case (1):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		break;
	case (2):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (3):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (4):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (5):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (6):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (7):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		break;
	case (8):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	case (9):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		break;
	}
	if (point)
		HAL_GPIO_WritePin(GPIOB, Point_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, Point_Pin, GPIO_PIN_RESET);

}

int write_num_display(int fila, int columna) {
	int num_hex = matriz_teclado[fila - 1][columna - 1];
	switch (num_hex) {
	case (0x0):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		return (0x0);
		break;
	case (0x1):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		return (0x1);
		break;
	case (0x2):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x2);
		break;
	case (0x3):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x3);
		break;
	case (0x4):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x4);
		break;
	case (0x5):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x5);
		break;
	case (0x6):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x6);
		break;
	case (0x7):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		return (0x7);
		break;
	case (0x8):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x8);
		break;
	case (0x9):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0x9);
		break;
	case (0xA):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0xA);
		break;
	case (0xB):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0xB);
		break;
	case (0xC):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		return (0xC);
		break;
	case (0xD):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0xD);
		break;
	case (0xE):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0xE);
		break;
	case (0xF):
		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);
		return (0xF);
		break;
	}
	return (-1);
}

int write_Display(int display, int fila, int columna) {

	switch (display) {
	case (1):
		HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_RESET);
		break;
	case (2):
		HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_RESET);
		break;
	case (3):
		HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_RESET);
		break;
	case (4):
		HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_RESET);
		break;
	}

	return (write_num_display(fila, columna));
}

int	count_digits(unsigned int decnum)
{
	int cnt = 0;
	while (decnum != 0)
	{
		cnt++;
		decnum /= 10;
	}
	return (cnt);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */
	//__HAL_RCC_GPIOA_CLK_ENABLE();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	/*****************************
	 *
	 *  LECTURA TECLADO MATRICIAL
	 *
	 *****************************/

	int columna;
	int fila;
	int d;
	long long int hexnum;
	unsigned int	hexnum_pequenyito;
	unsigned int decnum;
	int multiplicador;
	int hex_fil[8] = { -1 };
	int hex_col[8] = { -1 };
	int dec_digits[10] = { -1 };
	int index_digits;
	int tam_array; // El tamaño del array al final lo que indica es el número de dígitos de nuestro número hexadecimal
	int boton_matricial;
	int err;
	int tam_dec_digits;
	int leds;
	int limit_right;
	int limit_left;
	int point;

	while (1) {

		d = 4;
		hexnum = 0x0;
		decnum = 0;
		multiplicador = 1;
		columna = 0;
		fila = 0;
		boton_matricial = 0;
		tam_array = 0;
		index_digits = 0;
		for(int i = 0; i < 10; ++i)
			dec_digits[i] = -1;

		HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Point_Pin, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOC, Led4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Led3_Pin, GPIO_PIN_RESET);


		HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);

		// Lectura teclado matricial
		while (1) {
			HAL_Delay(500);
			boton_matricial = 0;
			while (HAL_GPIO_ReadPin(GPIOA, Button2_Pin) == 0) {
				if (HAL_GPIO_ReadPin(GPIOA, Button1_Pin) == 1)
					NVIC_SystemReset();

				HAL_GPIO_WritePin(GPIOC, Fil2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil1_Pin, GPIO_PIN_SET);

				columna = lectura_columnas();
				HAL_Delay(50);
				if (columna != 0) {
					fila = 1;
					HAL_GPIO_WritePin(GPIOC, Fil1_Pin, GPIO_PIN_RESET);
					boton_matricial = 1;
					break;
				}

				HAL_GPIO_WritePin(GPIOC, Fil1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil2_Pin, GPIO_PIN_SET);

				columna = lectura_columnas();
				HAL_Delay(50);
				if (columna != 0) {
					fila = 2;
					HAL_GPIO_WritePin(GPIOC, Fil2_Pin, GPIO_PIN_RESET);
					boton_matricial = 1;
					break;
				}

				HAL_GPIO_WritePin(GPIOC, Fil1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil3_Pin, GPIO_PIN_SET);

				columna = lectura_columnas();
				HAL_Delay(50);
				if (columna != 0) {
					fila = 3;
					HAL_GPIO_WritePin(GPIOC, Fil3_Pin, GPIO_PIN_RESET);
					boton_matricial = 1;
					break;
				}

				HAL_GPIO_WritePin(GPIOC, Fil1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Fil4_Pin, GPIO_PIN_SET);

				columna = lectura_columnas();
				HAL_Delay(50);
				if (columna != 0) {
					fila = 4;
					HAL_GPIO_WritePin(GPIOC, Fil4_Pin, GPIO_PIN_RESET);
					boton_matricial = 1;
					break;
				}
			}
			if (!boton_matricial) {
				break;
			}
			if (!d) { // Hay más digitos que displays en  el tester
				hexnum = hexnum * 0x10 + write_Display(1, fila, columna);
				HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);
				for (int i = 2; i <= 4; i++) {
					write_Display(i, hex_fil[tam_array - 1 - (i - 2)],
							hex_col[tam_array - 1 - (i - 2)]);
					HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);
				}
				hex_fil[tam_array] = fila;
				hex_col[tam_array] = columna;
				fila = 0;
				columna = 0;
				tam_array++;

			} else {
				hexnum = hexnum * 0x10 + write_Display(d, fila, columna);
				HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);
				hex_fil[tam_array] = fila;
				hex_col[tam_array] = columna;
				fila = 0;
				columna = 0;
				d--;
				tam_array++;
			}
		}

		if (hexnum <= INT_MAX) {
			hexnum_pequenyito = (unsigned int) hexnum;
			err = 0;
			HAL_Delay(300);

			while (hexnum_pequenyito != 0) {
				int digito = hexnum_pequenyito % 16; // Obtener el dígito hexadecimal menos significativo
				decnum += digito * multiplicador; // Agregar el dígito al resultado
				multiplicador *= 16; // Actualizar el multiplicador para el siguiente dígito
				hexnum_pequenyito /= 16; // Ir al siguiente dígito hexadecimal
			}

			tam_dec_digits = count_digits(decnum);
			// Introduzco los dígitos en el arr;ay
		for (int i = 0; i <= tam_dec_digits; i++)
				dec_digits[i] = (int) ((decnum / (unsigned int)pow(10, i)) % 10);

		} else { // Imprimir error
			err = 1;
			HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOB, Seg0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, Seg3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg4_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg5_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg6_Pin, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);
		}

		leds = 0;
		limit_right = 1;
		limit_left = 0;
		point = 0;
		index_digits = 0;
		while (!HAL_GPIO_ReadPin(GPIOA, Button1_Pin)) {
			if ((HAL_GPIO_ReadPin(GPIOC, Button3_Pin)) && !limit_left) //led izquierda
			{
				HAL_Delay(300);
				leds = 0;
				index_digits++;
			}
			if ((HAL_GPIO_ReadPin(GPIOC, Button4_Pin)) && !limit_right) //led derecha
			{
				HAL_Delay(300);
				leds = 0;
				index_digits--;
			}
			if (!err && !leds) {
				leds = 1;
				// Imprimo dígitos
				for (int display = 1; display <= 4; ++display) {
					switch (display) {
					case (1):
						HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
						break;
					case (2):
						HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
						break;
					case (3):
						HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_SET);
						break;
					case (4):
						HAL_GPIO_WritePin(GPIOC, Display1_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC, Display2_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, Display3_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOA, Display4_Pin, GPIO_PIN_RESET);
						break;
					}
					if (index_digits % 3 == 0 && index_digits != 0 )
						point = 1;
					else
						point = 0;
					write_num_display_dec(dec_digits[index_digits++], point);
					HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOA, CLK_Pin, GPIO_PIN_RESET);

				}
				if (index_digits == 4) { // Limite derecha
					HAL_GPIO_WritePin(GPIOC, Led4_Pin, GPIO_PIN_SET);
					limit_right = 1;
				} else
				{
					limit_right = 0;
					HAL_GPIO_WritePin(GPIOC, Led4_Pin, GPIO_PIN_RESET);
				}
				if (index_digits == tam_dec_digits || tam_dec_digits <= 4) { // Limite izquierda
					HAL_GPIO_WritePin(GPIOB, Led3_Pin, GPIO_PIN_SET);
					limit_left = 1;
				} else
				{
					limit_left = 0;
					HAL_GPIO_WritePin(GPIOB, Led3_Pin, GPIO_PIN_RESET);
				}
				index_digits -= 4;
				// Mover digitos izquierda o derecha
			}
		}

	}
}

	/**
	 * @brief System Clock Configuration
	 * @retval None
	 */
	void SystemClock_Config(void) {
		RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
		RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

		/** Configure the main internal regulator output voltage
		 */
		if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
				!= HAL_OK) {
			Error_Handler();
		}

		/** Initializes the RCC Oscillators according to the specified parameters
		 * in the RCC_OscInitTypeDef structure.
		 */
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
		RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
		RCC_OscInitStruct.PLL.PLLM = 1;
		RCC_OscInitStruct.PLL.PLLN = 10;
		RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
		RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
		RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/** Initializes the CPU, AHB and APB buses clocks
		 */
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
				| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4)
				!= HAL_OK) {
			Error_Handler();
		}
	}

	/**
	 * @brief USART2 Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_USART2_UART_Init(void) {

		/* USER CODE BEGIN USART2_Init 0 */

		/* USER CODE END USART2_Init 0 */

		/* USER CODE BEGIN USART2_Init 1 */

		/* USER CODE END USART2_Init 1 */
		huart2.Instance = USART2;
		huart2.Init.BaudRate = 115200;
		huart2.Init.WordLength = UART_WORDLENGTH_8B;
		huart2.Init.StopBits = UART_STOPBITS_1;
		huart2.Init.Parity = UART_PARITY_NONE;
		huart2.Init.Mode = UART_MODE_TX_RX;
		huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart2.Init.OverSampling = UART_OVERSAMPLING_16;
		huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		if (HAL_UART_Init(&huart2) != HAL_OK) {
			Error_Handler();
		}
		/* USER CODE BEGIN USART2_Init 2 */

		/* USER CODE END USART2_Init 2 */

	}

	/**
	 * @brief GPIO Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_GPIO_Init(void) {
		GPIO_InitTypeDef GPIO_InitStruct = { 0 };

		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOC,
				Display1_Pin | Display2_Pin | Fil2_Pin | Fil1_Pin | Led4_Pin
						| Fil4_Pin | Fil3_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOA, CLK_Pin | Display4_Pin | Seg3_Pin,
				GPIO_PIN_RESET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOB,
				Display3_Pin | Point_Pin | Seg2_Pin | Seg4_Pin | Seg5_Pin
						| Seg6_Pin | Seg1_Pin | Seg0_Pin | Led3_Pin,
				GPIO_PIN_RESET);

		/*Configure GPIO pin : B1_Pin */
		GPIO_InitStruct.Pin = B1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

		/*Configure GPIO pins : Display1_Pin Display2_Pin Fil2_Pin Fil1_Pin
		 Led4_Pin Fil4_Pin Fil3_Pin */
		GPIO_InitStruct.Pin = Display1_Pin | Display2_Pin | Fil2_Pin | Fil1_Pin
				| Led4_Pin | Fil4_Pin | Fil3_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/*Configure GPIO pins : CLK_Pin Display4_Pin Seg3_Pin */
		GPIO_InitStruct.Pin = CLK_Pin | Display4_Pin | Seg3_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/*Configure GPIO pins : Col4_Pin Col3_Pin Col2_Pin Button1_Pin
		 Button2_Pin */
		GPIO_InitStruct.Pin = Col4_Pin | Col3_Pin | Col2_Pin | Button1_Pin
				| Button2_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/*Configure GPIO pins : Button3_Pin Button4_Pin */
		GPIO_InitStruct.Pin = Button3_Pin | Button4_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/*Configure GPIO pins : Display3_Pin Point_Pin Seg2_Pin Seg4_Pin
		 Seg5_Pin Seg6_Pin Seg1_Pin Seg0_Pin
		 Led3_Pin */
		GPIO_InitStruct.Pin = Display3_Pin | Point_Pin | Seg2_Pin | Seg4_Pin
				| Seg5_Pin | Seg6_Pin | Seg1_Pin | Seg0_Pin | Led3_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/*Configure GPIO pin : Col1_Pin */
		GPIO_InitStruct.Pin = Col1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(Col1_GPIO_Port, &GPIO_InitStruct);

	}

	/* USER CODE BEGIN 4 */

	/* USER CODE END 4 */

	/**
	 * @brief  This function is executed in case of error occurrence.
	 * @retval None
	 */
	void Error_Handler(void) {
		/* USER CODE BEGIN Error_Handler_Debug */
		/* User can add his own implementation to report the HAL error return state */
		__disable_irq();
		while (1) {
		}
		/* USER CODE END Error_Handler_Debug */
	}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
