#ifndef __USART_H
#define __USART_H


#include <stdint.h>
#include <stdarg.h>


typedef void(*usart_receive_callback_t)(uint8_t data);


void Usart_Init(void);
void Usart_SendData(uint8_t *data, uint16_t length);
void Usart_SendString(char *str);
void Usart_Printf(char *format, ...);
void Usart_Receive_Register(usart_receive_callback_t callback);

#endif /* __USART_H  */
