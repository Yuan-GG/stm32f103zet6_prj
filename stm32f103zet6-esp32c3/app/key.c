#include <stdio.h>
#include "stm32f10x.h"
#include "main.h"
#include "key.h"


void key_init(void){
    /*   GPIO�ṹ�帳ֵ   */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

bool key_pressed(void){
    if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == KEY_DOWN){
        delayms(10);
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == KEY_DOWN){
            delayms(10);
            return true;
        }
    }
    return false;
}

void key_wait_release(void){
    while(key_pressed());
}
