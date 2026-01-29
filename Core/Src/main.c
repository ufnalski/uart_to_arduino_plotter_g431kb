/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include "ssd1306.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	CHECKSUM_OK = 0, CHECKSUM_ERROR,
} checksum_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define RX_BUFFER_SIZE 10
#define OLED_REFRESH_RATE 1000 // ms
#define DATA_SKIPPING_RATIO 2

//#define USE_ARDUINO_SERIAL_PLOTTER
//#define USE_SERIAL_PORT_PLOTTER
//#define USE_WEB_SERIAL_PLOTTER
//#define USE_BETTER_SERIAL_PLOTTER
#define USE_WEB_SERIAL_PLOTTER_2

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */

uint8_t rxData[RX_BUFFER_SIZE];
uint8_t rxDataBis[RX_BUFFER_SIZE];
uint8_t aux_msg_cnt = 0;
uint32_t if_msg_cnt = 0;
uint32_t all_msg_cnt = 0;
uint32_t checksum_error_cnt = 0;

uint32_t softTimerOLED;
char lcd_line[32];

volatile uint8_t rx_data_processed_flag = 1;
volatile uint16_t rx_data_size_bis;

float systick_time;

//union
//{
//	uint8_t uint8[2];
//	int16_t int16;
//} position_converter;

uint16_t right_wiper_position;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
checksum_t VerifyChecksum8(uint8_t *_data, uint8_t _payload_size);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

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
	MX_DMA_Init();
	MX_USART1_UART_Init();
	MX_I2C3_Init();
	/* USER CODE BEGIN 2 */

	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 2);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(10, 16);
	ssd1306_WriteString("UART to USB sniffer", Font_6x8, White);
	ssd1306_UpdateScreen();

	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, RX_BUFFER_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, RX_BUFFER_SIZE); // the first one is rarely effective - needs debugging

	/* USER CODE END 2 */

	/* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
	BspCOMInit.BaudRate = 115200;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits = COM_STOPBITS_1;
	BspCOMInit.Parity = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
	{
		Error_Handler();
	}

	/* USER CODE BEGIN BSP */

	/* -- Sample board code to send message over COM1 port ---- */
	printf("\r\nWelcome to UART to USB STM32 world!\r\n");

	/* USER CODE END BSP */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	softTimerOLED = HAL_GetTick();

	while (1)
	{
		if ((HAL_GetTick() - softTimerOLED) > OLED_REFRESH_RATE)
		{
			softTimerOLED = HAL_GetTick();
			sprintf(lcd_line, "All cnt:   %lu", all_msg_cnt);
			ssd1306_SetCursor(12, 32);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			sprintf(lcd_line, "If cnt:    %lu", if_msg_cnt);
			ssd1306_SetCursor(12, 44);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			sprintf(lcd_line, "Error cnt: %lu", checksum_error_cnt);
			ssd1306_SetCursor(12, 56);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_UpdateScreen();
		}

		if (rx_data_processed_flag == 0)
		{
			all_msg_cnt++;

			if (rx_data_size_bis == 5)
			{
				if_msg_cnt++;

				if (VerifyChecksum8(rxDataBis, 4) == CHECKSUM_ERROR)
				{
					checksum_error_cnt++;
				}

				aux_msg_cnt++;
				if ((aux_msg_cnt % DATA_SKIPPING_RATIO == 0)
						&& (VerifyChecksum8(rxDataBis, 4) == CHECKSUM_OK))
				{
					right_wiper_position = ((((uint16_t) rxDataBis[1]) << 8)
							| (uint16_t) rxDataBis[2]) >> 4;
#if defined(USE_ARDUINO_SERIAL_PLOTTER)	 // Arduino serial plotter (Arduino IDE)
					// https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-serial-plotter/
					printf("Wiper_position:%d\r\n", right_wiper_position);
#elif defined(USE_SERIAL_PORT_PLOTTER)  // Serial port plotter
					// https://github.com/CieNTi/serial_port_plotter
					printf("$%d;", right_wiper_position);
#elif defined(USE_WEB_SERIAL_PLOTTER)  // Web Serial Plotter (atomic14)
					// https://www.atomic14.com/2025/09/03/web-serial-plotter
					printf("%d\r\n", right_wiper_position);
#elif defined(USE_BETTER_SERIAL_PLOTTER)  // Better Serial Plotter
					// https://github.com/nathandunk/BetterSerialPlotter
					systick_time = ((float) HAL_GetTick()) / 1000.0f;
					printf("%.3f\t%d\r\n", systick_time, right_wiper_position);
#elif defined(USE_WEB_SERIAL_PLOTTER_2)  // Web Serial Plotter (sekigon-gonnoc)
					// https://github.com/sekigon-gonnoc/web-serial-plotter
					systick_time = ((float) HAL_GetTick()) / 1000.0f;
					printf("Systick_time:%.3f,Wiper_position:%d\r\n",
							systick_time, right_wiper_position);
#endif
					aux_msg_cnt = 0;
				}
			}

			rx_data_processed_flag = 1;
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
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

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART1)
	{
		if (rx_data_processed_flag == 1)
		{
			rx_data_processed_flag = 0;
			memcpy(rxDataBis, rxData, Size);
			rx_data_size_bis = Size;
		}
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, RX_BUFFER_SIZE);
	}
}

checksum_t VerifyChecksum8(uint8_t *_data, uint8_t _payload_size)
{
	uint8_t sum_of_bytes = 0;
	for (uint8_t i = 0; i < _payload_size; i++)
	{
		sum_of_bytes += _data[i];
	}

	if (sum_of_bytes == _data[_payload_size])
	{
		return CHECKSUM_OK;
	}
	else
	{
		return CHECKSUM_ERROR;
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
