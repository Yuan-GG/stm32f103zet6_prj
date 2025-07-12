#ifndef __KEY_H
#define __KEY_H


#include <stdbool.h>


#define KEY_PORT GPIOE
#define KEY_PIN GPIO_Pin_4
#define KEY_DOWN Bit_SET

void key_init(void);
bool key_pressed(void);
void key_wait_release(void);

#endif /* __KEY_H  */
