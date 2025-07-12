#include <stdio.h>
#include "stm32f10x.h"
#include "main.h"
#include "key.h"
#include "key_exti.h"


#define KEY_EXTI_PORT_SRC GPIO_PortSourceGPIOE
#define KEY_EXTI_PIN_SRC  GPIO_PinSource4
#define KEY_EXTI_LINE     EXTI_Line4


static key_press_callback_t key_press_callback;

void key_exti_init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Configure PE.04 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    
    /* Connect EXTI4 Line to PE.04 pin */
    GPIO_EXTILineConfig(KEY_EXTI_PORT_SRC, KEY_EXTI_PIN_SRC);

    /* Configure EXTI4 line */
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI4 Interrupt to the lowest priority */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void key_exti_register(key_press_callback_t callback){
    key_press_callback = callback;
}

// �ⲿ�ж���ñ�д����
void EXTI4_IRQHandler(void){
    if(EXTI_GetITStatus(KEY_EXTI_LINE) == SET){
        if(key_pressed()){
            if(key_press_callback){
                key_press_callback();
            }
            key_wait_release();
        }
        EXTI_ClearITPendingBit(KEY_EXTI_LINE);
    }
}
