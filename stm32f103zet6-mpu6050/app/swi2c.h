#ifndef __SWI2C_H__
#define __SWI2C_H__


#include <stdbool.h>
#include <stdint.h>


void SWI2C_Init(void);
bool SWI2C_Write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t length);
bool SWI2C_Read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t length);


#endif /* __SWI2C_H__ */
