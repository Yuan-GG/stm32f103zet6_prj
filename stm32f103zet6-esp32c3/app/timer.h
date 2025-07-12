#ifndef __TIMER_H
#define __TIMER_H
#include <stdbool.h>
#include <stdint.h>


typedef void(*timer_elapsed_callback_t)(void);

void timer_init(uint32_t period_ms);
void timer_start(void);
void timer_stop(void);
void timer_elapsed_register(timer_elapsed_callback_t cb);

void pwm_init(uint32_t period_ms);
void pwm_set_duty(uint32_t ch, uint32_t duty_us);
void pwm_start(void);
void pwm_stop(void);


#endif /* __TIMER_H  */
