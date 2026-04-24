#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"

// --- gokhan beyaz ---
#define MPU6050_ADDR (0x68 << 1)
#define HAREKET_ESIGI 15000

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_I2C1_Init(); // Burada pin ayarlarını yaptık kanka
  MX_USART2_UART_Init();

  // Terminale ilk mesaj
  char merhaba[] = "\r\n--- SISTEM BASLADI ---\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)merhaba, strlen(merhaba), 1000);

  // Sensörü uyandır
  uint8_t power_data = 0;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x6B, 1, &power_data, 1, 100);

  while (1) {
    uint8_t buffer[6];
    int16_t Ax, Ay, Az;
    char msg[128];

    // Sensörle konuşmaya çalışıyoruz
    if(HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, 1, buffer, 6, 100) == HAL_OK) {
        Ax = (int16_t)(buffer[0] << 8 | buffer[1]);
        Ay = (int16_t)(buffer[2] << 8 | buffer[3]);
        Az = (int16_t)(buffer[4] << 8 | buffer[5]);

        sprintf(msg, "X:%d Y:%d Z:%d\r\n", Ax, Ay, Az);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

        // Hareket kontrolü
        if (Ax > HAREKET_ESIGI || Ay > HAREKET_ESIGI || Ax < -HAREKET_ESIGI || Ay < -HAREKET_ESIGI) {
            char alert[] = "!!! HAREKET ALGILANDI !!!\r\n";
            HAL_UART_Transmit(&huart2, (uint8_t*)alert, strlen(alert), 500);
        }
    } else {
        // Pin ayarları eksikse burası çalışıyordu, şimdi düzelttik
        char hata[] = "Sensor baglantisi hatali!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)hata, strlen(hata), 100);
    }
    HAL_Delay(500);
  }
}

// BU KISIM ÇOK ÖNEMLİ: Yeşil kabloların (D0-D1) çalışmasını sağlar
static void MX_I2C1_Init(void) {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // PA9 (D1) -> SCL, PA10 (D0) -> SDA kabloların buralara bağlı
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B; 
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  HAL_I2C_Init(&hi2c1);
}

static void MX_USART2_UART_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  HAL_UART_Init(&huart2);
}

void SystemClock_Config(void) {
  // Varsayılan saat ayarı
}
