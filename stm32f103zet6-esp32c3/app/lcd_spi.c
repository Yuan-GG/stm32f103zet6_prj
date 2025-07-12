#include <stdio.h>
#include "stm32f10x.h"
#include "main.h"
#include "lcd_spi.h"

static lcd_spi_send_finish_callback_t lcd_spi_send_finish_callback;

static void lcd_spi_gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // SPI1_SCK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOA, GPIO_Pin_5 | GPIO_Pin_7, Bit_SET);
}

static void lcd_spi_nvic_init(void)
{
    // 使用DMA发送
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void lcd_spi_lowlevel_init(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);
 
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(SPI1, ENABLE);
}

/**
 * @brief  LCD_SPI初始化：GPIO-NVIC-SPI
 * @param  None
 * @retval None
 */
void lcd_spi_init(void)
{
    lcd_spi_gpio_init();
    lcd_spi_nvic_init();
    lcd_spi_lowlevel_init();
}

/**
 * @brief  LCD_SPI同步写，最后需要等BSY=RESET，避免最后一个字节没发送就关闭了
 * @param  data
 * @param  size
 * @retval None
 */
void lcd_spi_write_sync(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(SPI1, data[i]);
    }
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
}

/**
 * @brief  LCD_SPI异步写（DMA)
 * @param  data
 * @param  size
 * @retval None
 */
void lcd_spi_write_async(uint8_t *data, uint16_t size)
{
    DMA_Cmd(LCD_SPI_DMA_CHANNEL, DISABLE);
    DMA_DeInit(LCD_SPI_DMA_CHANNEL);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = SPI_DR_BASE;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(LCD_SPI_DMA_CHANNEL, &DMA_InitStructure);

    DMA_ITConfig(LCD_SPI_DMA_CHANNEL, DMA_IT_TC, ENABLE);
    DMA_Cmd(LCD_SPI_DMA_CHANNEL, ENABLE);
}

/**
 * @brief  注册回调函数
 * @param  cb-回调函数
 * @retval None
 */
void lcd_spi_send_finish_register(lcd_spi_send_finish_callback_t cb)
{
    lcd_spi_send_finish_callback = cb;
}

/**
 * @brief  DMA1中断
 * @param  None
 * @retval None
 */
void DMA1_Channel3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC3) == SET)
    {
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

        if (lcd_spi_send_finish_callback)
            lcd_spi_send_finish_callback();

        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
}
