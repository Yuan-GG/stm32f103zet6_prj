#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "usart.h"
#include "led.h"


#define USART_RX_PACKET_LEN 128


char usart_rxPacket[USART_RX_PACKET_LEN];
bool usart_rxFlag = false;
static usart_receive_callback_t usart_receive_callback;
static usart_send_finish_callback_t usart_send_finish_callback;

static void Usart_GPIO_Init(void){
    /* Configure GPIO */
    GPIO_InitTypeDef GPIO_InitStructure;
    // 鉴于这个PIN的default的AF就有USART_R/TX，所以不用remap
    // TX:发送--输出--驱动电平明确--推挽输出pp--又因为是复用所以为AF_PP
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);
    // RX:接收--输入--默认高电平（协议决定）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);
}
static void Usart_NVIC_Init(void){
    /* Configure USART NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Configure DMA NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
static void Usart_LowLevel_Init(void){
    /* Configure USART1 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
}

/**
  * @brief  初始化串口：GPIO-USART-DMA
  * @param  None
  * @retval None
  */
void Usart1_DMA_Tx_Init(void){
    
    Usart_GPIO_Init();
    Usart_NVIC_Init();
    Usart_LowLevel_Init();

    /* Enable USART1 Receive interrupts */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* Enable USART DMA Tx request */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    /* Enable the USART1 */
    USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  发送单字节整型data
  * @param  data，虽然需要uint16_t，但因为用的是USART_WordLength_8b，所以没问题
  * @param  None
  * @retval None
  */
static void Usart_SendByte(uint8_t data){
    // 写入TDR
    USART_SendData(USART1, data);
    // 等待硬件清零
    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
}

/**
  * @brief  发送整型data
  * @param  data，虽然需要uint16_t，但因为用的是USART_WordLength_8b，所以没问题
  * @param  length，长度
  * @retval None
  */
void Usart_SendData(uint8_t *data, uint16_t length){
    for (uint16_t i = 0; i < length; i++) {
        Usart_SendByte(data[i]);
    }
}

/**
  * @brief  发送字符串
  * @param  str
  * @retval None
  */
void Usart_SendString(char *str){
    uint16_t len = strlen(str);
    Usart_SendData((uint8_t *) str, len);
}

/**
  * @brief  发送格式化字符串
  * @param  format, ...
  * @retval None
  */
void Usart_Printf(char *format, ...){
    char str[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(str, format, arg);
    va_end(arg);
    Usart_SendString(str);
}

/**
  * @brief  使用DMA异步发送数据
  * @param  str
  * @retval None
  */
void Usart_DMA_Async_Init(uint8_t *data, uint16_t size){
    /* USART1_TX_DMA_CHANNEL configuration ---------------------------------*/
    DMA_Cmd(USART1_TX_DMA_CHANNEL, DISABLE);
    DMA_DeInit(USART1_TX_DMA_CHANNEL);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_BASE;
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
    DMA_Init(USART1_TX_DMA_CHANNEL, &DMA_InitStructure);
    /* Enable DMA IT */
    DMA_ITConfig(USART1_TX_DMA_CHANNEL, DMA_IT_TC, ENABLE);
    /* Enable DMA channels */
    DMA_Cmd(USART1_TX_DMA_CHANNEL, ENABLE);
}

void DMA_Transfer(uint16_t size) {
    DMA_Cmd(USART1_TX_DMA_CHANNEL, DISABLE);
    DMA_SetCurrDataCounter(USART1_TX_DMA_CHANNEL, size);
    DMA_Cmd(USART1_TX_DMA_CHANNEL, ENABLE);
    // 等待传输完成
    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC4); 
}

/**
  * @brief  注册回调函数
  * @param  usart_receive_callback_t
  * @retval None
  */
void Usart_Receive_Register(usart_receive_callback_t cb){
    usart_receive_callback = cb;
}

/**
  * @brief  注册回调函数
  * @param  usart_receive_callback_t
  * @retval None
  */
void Usart_send_finish_Register(usart_send_finish_callback_t cb){
    usart_send_finish_callback = cb;
}

void DMA1_Channel4_IRQHandler(void){
    if (DMA_GetITStatus(DMA1_IT_TC4) == SET){
        if (usart_send_finish_callback){
            usart_send_finish_callback();
        }
        // 清除*中断挂起*标志位
        DMA_ClearITPendingBit(DMA1_IT_TC4);
    }
}

/**
  * @brief  串口中断接收服务函数，分为单字节与String，需要在option中添加对应的宏定义
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void){
    
#ifdef USART_BYTE
    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET){
        /* Read one byte from the receive data register */
        uint8_t data = USART_ReceiveData(USART1);
        if(usart_receive_callback){
            usart_receive_callback(data);
        }
    }
    
#elif defined(USART_STRING)
    
    static UsartState currentState = STATE_HEAD;
    static uint8_t pRxPacket = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET){
        uint8_t rxData = USART_ReceiveData(USART1);
        
        if(currentState == STATE_HEAD){
            // 包头
            if(rxData == '@'){
                currentState = STATE_BODY;
                pRxPacket = 0;
            }
        } else if(currentState == STATE_BODY){
            if(rxData == '\r'){
                currentState = STATE_TAIL;
            } else{
                usart_rxPacket[pRxPacket] = rxData;
                pRxPacket++;
            }
        } else if(currentState == STATE_TAIL){
            if(rxData == '\n'){
                currentState = STATE_HEAD;
                usart_rxPacket[pRxPacket] = '\0';
                usart_rxFlag = true;
                Led_Toggle();
            }
        }
    }
                
#endif
}
    
/**
  * @brief  获取*包数据接收完成*标志位
  * @param  None
  * @retval None
  */
bool Usart_RxGetFlag(void){
    return usart_rxFlag;
}

/**
  * @brief  清除*包数据接收完成*标志位
  * @param  None
  * @retval None
  */
static void Usart_RxClearFlag(void){
    usart_rxFlag = false;
}

/**
  * @brief  发送*串口接收的包数据*
  * @param  None
  * @retval None
  */
void Usart_SendRxPacket(void){
    Usart_SendString(usart_rxPacket);
    Usart_RxClearFlag();
}
