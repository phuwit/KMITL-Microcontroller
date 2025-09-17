/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9341_Touchscreen.h"
#include "stdio.h"

#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"

#include "snow_tiger.h"
#include <stdint.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LIGHT_RED 0b1111110011010011
#define LIGHT_GREEN 0b1001111111110011
#define LIGHT_BLUE 0b1001110011011111

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

float redSaturation = 0;
float greenSaturation = 0;
float blueSaturation = 0;
uint16_t mixedColor = 0x0000;

uint8_t redValue = 0;
uint8_t greenValue = 0;
uint8_t blueValue = 0;

char temperaturePattern[] = "%.1fC";
char relHumidityPattern[] = "%.1f%%RH";
char saturationPattern[] = "%03d%%";
char temperatureBuffer[32] = "\0";
char relHumidityBuffer[32] = "\0";
char redSaturationBuffer[32] = "\0";
char greenSaturationBuffer[32] = "\0";
char blueSaturationBuffer[32] = "\0";

float h = 30.0, t = 40.0;
uint8_t cmd_buffer[3];
uint8_t data_buffer[8];
char environmentStringBuffer[32] = "\0";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void mixColor() {
  redValue = 0b11111;
  greenValue = 0b111111;
  blueValue = 0b11111;

  redValue = (uint8_t)(redValue * redSaturation);
  greenValue = (uint8_t)(greenValue * greenSaturation);
  blueValue = (uint8_t)(blueValue * blueSaturation);

  mixedColor = (uint16_t)(blueValue);
  mixedColor |= (greenValue << 5);
  mixedColor |= (redValue << 11);
}

uint16_t CRC16_2(uint8_t *ptr, uint8_t length) {
  uint16_t crc = 0xFFFF;
  uint8_t s = 0x00;

  while (length--) {
    crc ^= *ptr++;
    for (s = 0; s < 8; s++) {
      if ((crc & 0x01) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else
        crc >>= 1;
    }
  }

  return crc;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_RNG_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  cmd_buffer[0] = 0x03;
  cmd_buffer[1] = 0x00;
  cmd_buffer[2] = 0x04;

  ILI9341_Init(); // initial driver setup to drive ili9341

  ILI9341_Fill_Screen(WHITE);

  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

  ILI9341_Draw_Filled_Circle(120, 48, 24, BLACK);

  ILI9341_Draw_Filled_Circle(32, 112, 20, RED);
  ILI9341_Draw_Filled_Circle(32, 112 + 48, 20, GREEN);
  ILI9341_Draw_Filled_Circle(32, 112 + (48 * 2), 20, BLUE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    //	  HAL_Delay(5000);
    HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmd_buffer, 3, 200);
    //	  HAL_Delay(5);
    HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmd_buffer, 3, 200);
    HAL_Delay(5);
    HAL_I2C_Master_Receive(&hi2c1, 0x5c << 1, data_buffer, 8, 200);
    uint16_t Rcrc = data_buffer[7] << 8;
    Rcrc += data_buffer[6];
    if (Rcrc == CRC16_2(data_buffer, 6)) {
      uint16_t temperature = ((data_buffer[4] & 0x7F) << 8) + data_buffer[5];
      t = temperature / 10.0;
      t = (((data_buffer[4] & 0x80) >> 7) == 1) ? (t * (-1)) : t;
      uint16_t humidity = (data_buffer[2] << 8) + data_buffer[3];
      h = humidity / 10.0;
    }

    sprintf(environmentStringBuffer,
            "Temperature = %4.1f\tHumidity = %4.1f\n\r", t, h);
    HAL_UART_Transmit(&huart3, (uint8_t *)environmentStringBuffer,
                      strlen(environmentStringBuffer), 200);
    sprintf(environmentStringBuffer, "raw data: ");
    for (int i = 0; i < 8; i++) {
      char byteStr[6];
      sprintf(byteStr, "%02X ", data_buffer[i]);
      strcat(environmentStringBuffer, byteStr);
    }
    strcat(environmentStringBuffer, "\n\r");
    HAL_UART_Transmit(&huart3, (uint8_t *)environmentStringBuffer,
                      strlen(environmentStringBuffer), 200);

    ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
    if (TP_Touchpad_Pressed()) {
      uint16_t posX = 0;
      uint16_t posY = 0;
      uint16_t position_array[2];
      if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
        posX = position_array[0];
        posY = position_array[1];

        // ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
        // char counter_buff[30];
        // sprintf(counter_buff, "POS X: %.3d", posX);
        // ILI9341_Draw_Text(counter_buff, 10, 0, BLACK, 2, WHITE);
        // sprintf(counter_buff, "POS Y: %.3d", posY);
        // ILI9341_Draw_Text(counter_buff, 10, 16, BLACK, 2, WHITE);
        // ILI9341_Set_Rotation(SCREEN_VERTICAL_1);

        if ((posX >= 12 && posX <= 52) && (posY >= 16 && posY <= 56)) {
          blueSaturation += 0.1;
        }
        if ((posX >= 60 && posX <= 100) && (posY >= 16 && posY <= 56)) {
          greenSaturation += 0.1;
        }
        if ((posX >= 110 && posX <= 150) && (posY >= 16 && posY <= 56)) {
          redSaturation += 0.1;
        }
      }
    }
    ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

    if (redSaturation > 1.01)
      redSaturation = 0;
    if (greenSaturation > 1.01)
      greenSaturation = 0;
    if (blueSaturation > 1.01)
      blueSaturation = 0;

    sprintf(temperatureBuffer, temperaturePattern, (float)t);
    sprintf(relHumidityBuffer, relHumidityPattern, (float)h);
    sprintf(redSaturationBuffer, saturationPattern,
            (uint8_t)(redSaturation * 100));
    sprintf(greenSaturationBuffer, saturationPattern,
            (uint8_t)(greenSaturation * 100));
    sprintf(blueSaturationBuffer, saturationPattern,
            (uint8_t)(blueSaturation * 100));

    ILI9341_Draw_Filled_Circle(120, 48, 24, mixedColor);

    ILI9341_Draw_Rectangle(64, 92, 100, 40, LIGHT_RED);
    ILI9341_Draw_Rectangle(64, 92, (uint16_t)(100 * redSaturation), 40, RED);
    ILI9341_Draw_Rectangle(64, 140, 100, 40, LIGHT_GREEN);
    ILI9341_Draw_Rectangle(64, 140, (uint16_t)(100 * greenSaturation), 40,
                           GREEN);
    ILI9341_Draw_Rectangle(64, 188, 100, 40, LIGHT_BLUE);
    ILI9341_Draw_Rectangle(64, 188, (uint16_t)(100 * blueSaturation), 40, BLUE);

    ILI9341_Draw_Text(temperatureBuffer, 16, 40, BLACK, 2, WHITE);
    ILI9341_Draw_Text(relHumidityBuffer, 160, 40, BLACK, 2, WHITE);
    ILI9341_Draw_Text(redSaturationBuffer, 180, 100, BLACK, 2, WHITE);
    ILI9341_Draw_Text(greenSaturationBuffer, 180, 148, BLACK, 2, WHITE);
    ILI9341_Draw_Text(blueSaturationBuffer, 180, 196, BLACK, 2, WHITE);

    mixColor();

    HAL_Delay(200);
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
   */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
    Error_Handler();
  }
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
  while (1) {
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, tex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
