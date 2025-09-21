#include "ILI9341_CustomGFX.h"
#include "ILI9341_GFX.h"

#include "5x5_font.h"
#include "ILI9341_STM32_Driver.h"
#include "stm32f7xx_hal_spi.h"
#include <stdint.h>

void ILI9341_CustomGFX_DrawCustomSizedImage(
    const uint16_t originX, const uint16_t originY, const uint16_t width,
    const uint16_t height, const uint8_t *imageArray, uint8_t orientation) {
  ILI9341_Set_Rotation(orientation);

  ILI9341_Set_Address(originX, originY, originX + width - 1,
                      originY + height - 1);

  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

  unsigned char batchTransmitBuffer[BURST_MAX_SIZE];
  uint32_t counter = 0;
  for (uint32_t i = 0; i < width * height * 2 / BURST_MAX_SIZE; i++) {
    for (uint32_t k = 0; k < BURST_MAX_SIZE; k++) {
      batchTransmitBuffer[k] = imageArray[counter + k];
    }
    HAL_SPI_Transmit(&hspi5, (unsigned char *)batchTransmitBuffer,
                     BURST_MAX_SIZE, 10);
    counter += BURST_MAX_SIZE;
  }
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}