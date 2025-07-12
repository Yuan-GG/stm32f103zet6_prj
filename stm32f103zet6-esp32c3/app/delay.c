#include <stdint.h>
#include "main.h"
#include "stm32f10x.h"


void delayus(uint32_t us){
    SysTick->LOAD = us * (SystemCoreClock / 1000000) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

void delayms(uint32_t ms){
    while (ms--){
        delayus(1000);
    }
}

