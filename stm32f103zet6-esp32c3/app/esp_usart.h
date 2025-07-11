#ifndef __ESP_USART_H
#define __ESP_USART_H


#include <stdint.h>
#include <stdarg.h>


typedef void(*esp_usart_receive_callback_t)(uint8_t data);
typedef void(*esp_usart_send_finish_callback_t)(void);

typedef enum {
    STATE_HEAD,
    STATE_BODY,
    STATE_TAIL
} ESP_USARTState;

void ESP_USART2_Init(void);
void ESP_USART2_SendData(uint8_t *data, uint16_t length);
void ESP_USART2_SendString(const char* str);
void ESP_USART2_Printf(char *format, ...);
void ESP_USART2_DMA_Async_Init(uint8_t *data, uint16_t length);
void USART2_DMA_Transfer(uint16_t size);
void ESP_USART2_Receive_Register(esp_usart_receive_callback_t cb);
void ESP_USART2_Send_Finish_Register(esp_usart_send_finish_callback_t cb);
bool ESP_USART2_RxGetFlag(void);
static void ESP_USART2_RxClearFlag(void);
void ESP_USART2_SendRxPacket(void);

#endif /* __ESP_USART_H  */
