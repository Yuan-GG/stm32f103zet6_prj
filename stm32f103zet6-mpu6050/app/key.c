#include <stdio.h>
#include "stm32f10x.h"
#include "main.h"
#include "key.h"


void Key_Init(void){
    /*   GPIO结构体赋值   */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

bool Key_Pressed(void){
    if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == KEY_DOWN){
        delay(10);
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == KEY_DOWN){
            delay(10);
            return true;
        }
    }
    return false;
}

void Key_WaitRelease(void){
    while(Key_Pressed());
}
