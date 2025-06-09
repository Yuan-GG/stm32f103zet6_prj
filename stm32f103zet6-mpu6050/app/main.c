#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "led.h"
#include "key.h"
#include "key_exti.h"
#include "usart.h"
#include "st7735.h"
#include "mpu6050.h"
#include "swi2c.h"
#include "main.h"


//static uint16_t rxLen = 0;
volatile bool dma_tx_done = true;

static void Board_LowLevel_Init(){
    /* Enable RCC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    
    /* Enable USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* Enable SPI1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /* Enable AHB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    /* Configure NVIC priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

//static void On_Key_Pressed(void){
//    Led_Toggle();
//}

//static void On_Usart_Received(uint8_t data){
//    if(rxLen < RX_DATA_LEN - 1){
//        rxData[rxLen++] = data;
//        rxData[rxLen] = '\0';
//    }
//}

static void On_DMA_Tx_Finish(void){
    dma_tx_done = true;
}
    
int main(){
    Board_LowLevel_Init();
    
    Led_Init();
    Usart1_DMA_Tx_Init();
    Usart_send_finish_Register(On_DMA_Tx_Finish);
    ST7735_Init();
    MPU6050_Init();

    ST7735_Fill_Screen(ST7735_BLACK);

    char str[64];
    // uint8_t addr;
    // SWI2C_Read(0x68, 0x68, &addr, 1);
    // sprintf(str, "Addr: 0x%02X", addr);
    // ST7735_Write_String(0, 0, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);

    while(1){
        mpu6050_accel_t accel;
        mpu6050_gyro_t gyro;
        float temper;

        MPU6050_Read_Accel(&accel);
        MPU6050_Read_Gyro(&gyro);
        MPU6050_Read_Temper(&temper);

        sprintf(str, "Accel:");
        ST7735_Write_String(0, 0, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", accel.x);
        ST7735_Write_String(0, 16, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", accel.y);
        ST7735_Write_String(0, 32, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", accel.z);
        ST7735_Write_String(0, 48, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);

        sprintf(str, "Gyro:");
        ST7735_Write_String(64, 0, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", gyro.x);
        ST7735_Write_String(64, 16, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", gyro.y);
        ST7735_Write_String(64, 32, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
        sprintf(str, "%4.2f", gyro.z);
        ST7735_Write_String(64, 48, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);

        sprintf(str, "Temper: %4.2f", temper);
        ST7735_Write_String(0, 128 - 16, str, &font_ascii_8x16, ST7735_WHITE, ST7735_BLACK);
    }
}
