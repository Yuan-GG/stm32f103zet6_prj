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

void esp_usart2_init(void);
void esp_usart2_send_data(uint8_t *data, uint16_t length);
void esp_usart2_send_string(const char* str);
void esp_usart2_printf(char *format, ...);
void esp_usart2_receive_register(esp_usart_receive_callback_t cb);
void esp_usart2_send_finish_register(esp_usart_send_finish_callback_t cb);
bool esp_usart2_rx_getflag(void);
static void esp_usart2_rx_clearflag(void);
void esp_usart2_rx_sendpacket(void);

#endif /* __ESP_USART_H  */
