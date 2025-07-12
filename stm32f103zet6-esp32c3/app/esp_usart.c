#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "esp_usart.h"
#include "led.h"

#define ESP_USART1_PORT            GPIOA
#define ESP_USART1_TX_PIN          GPIO_Pin_9
#define ESP_USART1_RX_PIN          GPIO_Pin_10
#define ESP_USART1_DR_BASE         ((uint32_t)&(USART1->DR))
#define ESP_USART1_TX_DMA_CHANNEL  DMA1_Channel4


#define ESP_USART2_PORT            GPIOA
#define ESP_USART2_TX_PIN          GPIO_Pin_2
#define ESP_USART2_RX_PIN          GPIO_Pin_3
#define ESP_USART2_DR_BASE         ((uint32_t)&(USART2->DR))
#define ESP_USART2_TX_DMA_CHANNEL  DMA1_Channel4

// #define ESP_USART3_PORT            GPIOB
// #define ESP_USART3_TX_PIN          GPIO_Pin_10
// #define ESP_USART3_RX_PIN          GPIO_Pin_11
// #define ESP_USART3_DR_BASE         ((uint32_t)&(ESP_USART3->DR))

#define ESP_USART_RX_PACKET_LEN 128


char esp_usart_rxPacket[ESP_USART_RX_PACKET_LEN];
bool esp_usart_rxFlag = false;
static esp_usart_receive_callback_t esp_usart_receive_callback;
static esp_usart_send_finish_callback_t esp_usart_send_finish_callback;

static void esp_usart2_gpio_init(void) {
    /* Configure GPIO */
    GPIO_InitTypeDef GPIO_InitStructure;
    // 鉴于这个PIN的default的AF就有ESP_USART_R/TX，所以不用remap
    // TX:发送--输出--驱动电平明确--推挽输出pp--又因为是复用所以为AF_PP
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = ESP_USART2_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ESP_USART2_PORT, &GPIO_InitStructure);
    // RX:接收--输入--默认高电平（协议决定）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = ESP_USART2_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ESP_USART2_PORT, &GPIO_InitStructure);
}
static void esp_usart2_nvic_init(void) {
    /* Configure ESP_USART NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void esp_usart2_lowlevel_init(void) {
    /* Configure USART2 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
}

/**
  * @brief  初始化串口：GPIO-USART-DMA
  * @param  None
  * @retval None
  */
void esp_usart2_init(void) {

    esp_usart2_gpio_init();
    esp_usart2_nvic_init();
    esp_usart2_lowlevel_init();

    /* Enable USART2 Receive interrupts */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    /* Enable the USART2 */
    USART_Cmd(USART2, ENABLE);
}

/**
  * @brief  发送单字节整型data
  * @param  data，虽然需要uint16_t，但因为用的是USART_WordLength_8b，所以没问题
  * @param  None
  * @retval None
  */
static void esp_usart2_send_byte(uint8_t data) {
    // 写入TDR
    USART_SendData(USART2, data);
    // 等待硬件清零
    while (!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
}

/**
  * @brief  发送整型data
  * @param  data，虽然需要uint16_t，但因为用的是USART_WordLength_8b，所以没问题
  * @param  length，长度
  * @retval None
  */
void esp_usart2_send_data(uint8_t* data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        esp_usart2_send_byte(data[i]);
    }
}

/**
  * @brief  发送字符串
  * @param  str
  * @retval None
  */
void esp_usart2_send_string(const char* str) {
    uint16_t len = strlen(str);
    esp_usart2_send_data((uint8_t*)str, len);
}

/**
  * @brief  发送格式化字符串
  * @param  format, ...
  * @retval None
  */
void esp_usart2_printf(char* format, ...) {
    char str[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(str, format, arg);
    va_end(arg);
    esp_usart2_send_string(str);
}

/**
  * @brief  注册回调函数
  * @param  esp_usart_receive_callback_t
  * @retval None
  */
void esp_usart2_receive_register(esp_usart_receive_callback_t cb) {
    esp_usart_receive_callback = cb;
}

/**
  * @brief  注册回调函数
  * @param  esp_usart_receive_callback_t
  * @retval None
  */
void esp_usart2_send_finish_register(esp_usart_send_finish_callback_t cb) {
    esp_usart_send_finish_callback = cb;
}

void DMA1_Channel4_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC4) == SET) {
        if (esp_usart_send_finish_callback) {
            esp_usart_send_finish_callback();
        }
        // 清除*中断挂起*标志位
        DMA_ClearITPendingBit(DMA1_IT_TC4);
    }
}

/**
  * @brief  串口中断接收服务函数，分为单字节与String，需要在option中添加对应的宏定义
  *         数据包格式的中断
  * @param  None
  * @retval None
  */
// void USART2_IRQHandler(void) {

// #ifdef ESP_USART_BYTE

//     if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
//         /* Read one byte from the receive data register */
//         uint8_t data = ESP_USART_ReceiveData(USART2);
//         if (esp_usart_receive_callback) {
//             esp_usart_receive_callback(data);
//         }
//     }

// #elif defined(ESP_USART_STRING)

//     static ESP_USARTState currentState = STATE_HEAD;
//     static uint8_t pRxPacket = 0;
//     if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
//         uint8_t rxData = USART_ReceiveData(USART2);

//         if (currentState == STATE_HEAD) {
//             // 包头
//             if (rxData == '@') {
//                 currentState = STATE_BODY;
//                 pRxPacket = 0;
//             }
//         }
//         else if (currentState == STATE_BODY) {
//             if (rxData == '\r') {
//                 currentState = STATE_TAIL;
//             }
//             else {
//                 esp_usart_rxPacket[pRxPacket] = rxData;
//                 pRxPacket++;
//             }
//         }
//         else if (currentState == STATE_TAIL) {
//             if (rxData == '\n') {
//                 currentState = STATE_HEAD;
//                 esp_usart_rxPacket[pRxPacket] = '\0';
//                 esp_usart_rxFlag = true;
//                 Led_Toggle();
//             }
//         }
//     }

// #endif
// }

/**
  * @brief  串口中断接收服务函数，分为单字节与String，需要在option中添加对应的宏定义
  *         一般数据的中断
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t data = USART_ReceiveData(USART2);
        
        if (esp_usart_receive_callback)
            esp_usart_receive_callback(data);
    }
}

/**
  * @brief  获取*包数据接收完成*标志位
  * @param  None
  * @retval None
  */
bool esp_usart2_rx_getflag(void) {
    return esp_usart_rxFlag;
}

/**
  * @brief  清除*包数据接收完成*标志位
  * @param  None
  * @retval None
  */
static void esp_usart2_rx_clearflag(void) {
    esp_usart_rxFlag = false;
}

/**
  * @brief  发送*串口接收的包数据*
  * @param  None
  * @retval None
  */
void esp_usart2_rx_sendpacket(void) {
    esp_usart2_send_string(esp_usart_rxPacket);
    esp_usart2_rx_clearflag();
}
