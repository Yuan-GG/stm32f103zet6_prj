#ifndef __KEY_H
#define __KEY_H


#include <stdbool.h>


#define KEY_PORT GPIOE
#define KEY_PIN GPIO_Pin_4
#define KEY_DOWN Bit_SET

void Key_Init(void);
bool Key_Pressed(void);
void Key_WaitRelease(void);

#endif /* __KEY_H  */
