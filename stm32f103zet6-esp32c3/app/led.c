#include <stdio.h>
#include "stm32f10x.h"
#include "led.h"


#define LED_PORT GPIOB
#define LED_PIN GPIO_Pin_5


static bool led_state = false;

void Led_Init(void){
    /*   GPIO结构体赋值   */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    GPIO_SetBits(LED_PORT, LED_PIN);
}

void Led_Toggle(void){
    led_state = !led_state;
    GPIO_WriteBit(LED_PORT, LED_PIN, led_state ? Bit_RESET : Bit_SET);
}


