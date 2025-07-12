#ifndef __LCD_SPI_H
#define __LCD_SPI_H


#include <stdbool.h>


#define LCD_SPI_DMA_CHANNEL DMA1_Channel3
#define SPI_DR_BASE         ((uint32_t)&SPI1->DR)


typedef void(* lcd_spi_send_finish_callback_t)();
void lcd_spi_init(void);
void lcd_spi_write_sync(uint8_t *data, uint16_t size);
void lcd_spi_write_async(uint8_t *data, uint16_t size);
void lcd_spi_send_finish_register(lcd_spi_send_finish_callback_t cb);

#endif /* __LCD_SPI_H  */
