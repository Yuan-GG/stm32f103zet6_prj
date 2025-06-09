#ifndef __USART_H
#define __USART_H


#include <stdint.h>
#include <stdarg.h>


#define USART1_PORT            GPIOA
#define USART1_TX_PIN          GPIO_Pin_9
#define USART1_RX_PIN          GPIO_Pin_10
#define USART1_DR_BASE         ((uint32_t)&(USART1->DR))
#define USART1_TX_DMA_CHANNEL  DMA1_Channel4


#define USART2_PORT            GPIOA
#define USART2_TX_PIN          GPIO_Pin_2
#define USART2_RX_PIN          GPIO_Pin_3
#define USART2_DR_BASE         ((uint32_t)&(USART2->DR))

// #define USART3_PORT            GPIOB
// #define USART3_TX_PIN          GPIO_Pin_10
// #define USART3_RX_PIN          GPIO_Pin_11
// #define USART3_DR_BASE         ((uint32_t)&(USART3->DR))


typedef void(*usart_receive_callback_t)(uint8_t data);
typedef void(*usart_send_finish_callback_t)(void);

typedef enum {
    STATE_HEAD,
    STATE_BODY,
    STATE_TAIL
} UsartState;

void Usart1_DMA_Tx_Init(void);
void Usart_SendData(uint8_t *data, uint16_t length);
void Usart_SendString(char *str);
void Usart_Printf(char *format, ...);
void Usart_DMA_Async_Init(uint8_t *data, uint16_t length);
void DMA_Transfer(uint16_t size);
void Usart_Receive_Register(usart_receive_callback_t cb);
void Usart_send_finish_Register(usart_send_finish_callback_t cb);
bool Usart_RxGetFlag(void);
static void Usart_RxClearFlag(void);
void Usart_SendRxPacket(void);

#endif /* __USART_H  */
