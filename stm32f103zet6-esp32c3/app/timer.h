#ifndef __TIMER_H
#define __TIMER_H
#include <stdbool.h>
#include <stdint.h>


typedef void(*timer_elapsed_callback_t)(void);

void Timer_Init(uint32_t period_ms);
void Timer_Start(void);
void Timer_Stop(void);
void Timer_Elapsed_Register(timer_elapsed_callback_t cb);

void PWM_Init(uint32_t period_ms);
void PWM_SetDuty(uint32_t ch, uint32_t duty_us);
void PWM_Start(void);
void PWM_Stop(void);


#endif /* __TIMER_H  */
