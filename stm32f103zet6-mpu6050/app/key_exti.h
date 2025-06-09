#ifndef __KEY_EXTI_H
#define __KEY_EXTI_H


#include <stdbool.h>


typedef void (*key_press_callback_t) (void);

void Key_Exti_Init(void);
void Key_Exti_Register(key_press_callback_t callback);

#endif /* __KEY_EXTI_H  */
