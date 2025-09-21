/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_Touchscreen.h"

#include "image.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LIGHT_RED 0b1111110011010011
#define LIGHT_GREEN 0b1001111111110011
#define LIGHT_BLUE 0b1001110011011111

#define ADC1_VALUES_COUNT 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x2007c000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x2007c0a0
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x2007c000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x2007c0a0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ETH_HandleTypeDef heth;

I2C_HandleTypeDef hi2c1;

RNG_HandleTypeDef hrng;

SPI_HandleTypeDef hspi5;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

uint16_t posX = 0;
uint16_t posY = 0;
uint16_t position_array[2];

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

float h = 99;
float t = 99;
float hPrev = 0;
float tPrev = 0;
uint8_t cmd_buffer[3];
uint8_t data_buffer[8];
char environmentStringBuffer[64] = "\0";

uint8_t screen = 0;

char infoScreenLine1[] = "Group #38";
char infoScreenLine2[] = "Phuwit";
char infoScreenLine3[] = "Puthipairoj";
char infoScreenLine4[] = "66010648";

uint32_t adc1Values[ADC1_VALUES_COUNT];

float brightness;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_RNG_Init(void);
static void MX_SPI5_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

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

void initializeScreen0() {
  ILI9341_Fill_Screen(WHITE);

  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

  ILI9341_Draw_Filled_Circle(120, 48, 24, mixedColor);

  ILI9341_Draw_Filled_Circle(32, 112, 20, RED);
  ILI9341_Draw_Filled_Circle(32, 112 + 48, 20, GREEN);
  ILI9341_Draw_Filled_Circle(32, 112 + (48 * 2), 20, BLUE);

  ILI9341_Draw_Rectangle(64, 188, 100, 40, LIGHT_BLUE);
  ILI9341_Draw_Rectangle(64, 188, (uint16_t)(100 * blueSaturation), 40, BLUE);
  ILI9341_Draw_Rectangle(64, 140, 100, 40, LIGHT_GREEN);
  ILI9341_Draw_Rectangle(64, 140, (uint16_t)(100 * greenSaturation), 40, GREEN);
  ILI9341_Draw_Rectangle(64, 92, 100, 40, LIGHT_RED);
  ILI9341_Draw_Rectangle(64, 92, (uint16_t)(100 * redSaturation), 40, RED);
  sprintf(blueSaturationBuffer, saturationPattern,
          (uint8_t)(blueSaturation * 100));
  ILI9341_Draw_Text(blueSaturationBuffer, 180, 196, BLACK, 2, WHITE);
  sprintf(greenSaturationBuffer, saturationPattern,
          (uint8_t)(greenSaturation * 100));
  ILI9341_Draw_Text(greenSaturationBuffer, 180, 148, BLACK, 2, WHITE);
  sprintf(redSaturationBuffer, saturationPattern,
          (uint8_t)(redSaturation * 100));
  ILI9341_Draw_Text(redSaturationBuffer, 180, 100, BLACK, 2, WHITE);
}

// void updateBrightness() {
//   brightness =
//       (((float)adc1Values[0] / (float)(uint32_t)0x00000fff) * 0.8f) + 0.2f;

//   htim3.Instance->CCR1 = (10000 - 1) * brightness;
// }
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_ETH_Init();
  MX_RNG_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1Values, ADC1_VALUES_COUNT);

  cmd_buffer[0] = 0x03;
  cmd_buffer[1] = 0x00;
  cmd_buffer[2] = 0x04;

  ILI9341_Init(); // initial driver setup to drive ili9341

  initializeScreen0();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (screen == 0) {
      //	  HAL_Delay(5000);
      HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmd_buffer, 3, 200);
      //	  HAL_Delay(5);
      HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmd_buffer, 3, 200);
      HAL_Delay(5);
      HAL_I2C_Master_Receive(&hi2c1, 0x5c << 1, data_buffer, 8, 200);
      uint16_t Rcrc = data_buffer[7] << 8;
      Rcrc += data_buffer[6];

      hPrev = h;
      tPrev = t;

      if (Rcrc == CRC16_2(data_buffer, 6)) {
        uint16_t temperature = ((data_buffer[4] & 0x7F) << 8) + data_buffer[5];
        t = temperature / 10.0;
        t = (((data_buffer[4] & 0x80) >> 7) == 1) ? (t * (-1)) : t;
        uint16_t humidity = (data_buffer[2] << 8) + data_buffer[3];
        h = humidity / 10.0;
      }

      // sprintf(environmentStringBuffer,
      //         "Temperature = %4.1f\tHumidity = %4.1f\n\r", t, h);
      // HAL_UART_Transmit(&huart3, (uint8_t *)environmentStringBuffer,
      //                   strlen(environmentStringBuffer), 200);
      // sprintf(environmentStringBuffer, "raw data: ");
      // for (int i = 0; i < 8; i++) {
      //   char byteStr[6];
      //   sprintf(byteStr, "%02X ", data_buffer[i]);
      //   strcat(environmentStringBuffer, byteStr);
      // }
      // strcat(environmentStringBuffer, "\n\r");
      // HAL_UART_Transmit(&huart3, (uint8_t *)environmentStringBuffer,
      //                   strlen(environmentStringBuffer), 200);

      ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
      if (tPrev != t) {
        sprintf(temperatureBuffer, temperaturePattern, (float)t);
        ILI9341_Draw_Text(temperatureBuffer, 16, 40, BLACK, 2, WHITE);
      }
      if (hPrev != h) {
        sprintf(relHumidityBuffer, relHumidityPattern, (float)h);
        ILI9341_Draw_Text(relHumidityBuffer, 160, 40, BLACK, 2, WHITE);
      }

      ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
      if (TP_Touchpad_Pressed()) {
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

          ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
          if ((posX >= 12 && posX <= 52) && (posY >= 16 && posY <= 56)) {
            blueSaturation += 0.1;
            if (blueSaturation > 1.01)
              blueSaturation = 0;
            sprintf(blueSaturationBuffer, saturationPattern,
                    (uint8_t)(blueSaturation * 100));
            ILI9341_Draw_Rectangle(64, 188, 100, 40, LIGHT_BLUE);
            ILI9341_Draw_Rectangle(64, 188, (uint16_t)(100 * blueSaturation),
                                   40, BLUE);
            ILI9341_Draw_Text(blueSaturationBuffer, 180, 196, BLACK, 2, WHITE);
          }
          if ((posX >= 60 && posX <= 100) && (posY >= 16 && posY <= 56)) {
            greenSaturation += 0.1;
            if (greenSaturation > 1.01)
              greenSaturation = 0;
            sprintf(greenSaturationBuffer, saturationPattern,
                    (uint8_t)(greenSaturation * 100));
            ILI9341_Draw_Rectangle(64, 140, 100, 40, LIGHT_GREEN);
            ILI9341_Draw_Rectangle(64, 140, (uint16_t)(100 * greenSaturation),
                                   40, GREEN);
            ILI9341_Draw_Text(greenSaturationBuffer, 180, 148, BLACK, 2, WHITE);
          }
          if ((posX >= 110 && posX <= 150) && (posY >= 16 && posY <= 56)) {
            redSaturation += 0.1;
            if (redSaturation > 1.01)
              redSaturation = 0;
            sprintf(redSaturationBuffer, saturationPattern,
                    (uint8_t)(redSaturation * 100));
            ILI9341_Draw_Rectangle(64, 92, 100, 40, LIGHT_RED);
            ILI9341_Draw_Rectangle(64, 92, (uint16_t)(100 * redSaturation), 40,
                                   RED);
            ILI9341_Draw_Text(redSaturationBuffer, 180, 100, BLACK, 2, WHITE);
          }
          if ((posX >= 172 && posX <= 220) && (posY >= 96 && posY <= 144)) {
            screen = 1;
            continue;
          }

          mixColor();
          ILI9341_Draw_Filled_Circle(120, 48, 24, mixedColor);
        }
      }

    } else if (screen == 1) {
      ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

      ILI9341_Draw_Image(CHILL_GUY, SCREEN_HORIZONTAL_1);

      ILI9341_Draw_Text(infoScreenLine1, 144, 64 + (32 * 0), mixedColor, 2,
                        WHITE);
      ILI9341_Draw_Text(infoScreenLine2, 144, 64 + (32 * 1), mixedColor, 2,
                        WHITE);
      ILI9341_Draw_Text(infoScreenLine3, 144, 64 + (32 * 2), mixedColor, 2,
                        WHITE);
      ILI9341_Draw_Text(infoScreenLine4, 144, 64 + (32 * 3), mixedColor, 2,
                        WHITE);

      HAL_TIM_Base_Start_IT(&htim2);

      while (screen == 1) {
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

          if ((posX >= 28 && posX <= 226) && (posY >= 16 && posY <= 134)) {
            screen = 0;
            initializeScreen0();
            break;
          }
        }
      }

      // screen = 0;
      // initializeScreen0();
    }

    // HAL_Delay(200);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x20303E5D;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI5_Init(void)
{

  /* USER CODE BEGIN SPI5_Init 0 */

  /* USER CODE END SPI5_Init 0 */

  /* USER CODE BEGIN SPI5_Init 1 */

  /* USER CODE END SPI5_Init 1 */
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 7;
  hspi5.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi5.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI5_Init 2 */

  /* USER CODE END SPI5_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 10000-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 108-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 5000000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, T_CLK_Pin|T_MISO_Pin|T_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CS_Pin|DC_Pin|RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : T_IRQ_Pin T_MOSI_Pin */
  GPIO_InitStruct.Pin = T_IRQ_Pin|T_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : T_CLK_Pin T_MISO_Pin T_CS_Pin */
  GPIO_InitStruct.Pin = T_CLK_Pin|T_MISO_Pin|T_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin DC_Pin RST_Pin */
  GPIO_InitStruct.Pin = CS_Pin|DC_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) { updateBrightness(); }

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
