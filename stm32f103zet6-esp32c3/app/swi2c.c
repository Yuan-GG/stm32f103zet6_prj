#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "swi2c.h"


#define SCL_PORT    GPIOB
#define SCL_PIN     GPIO_Pin_10
#define SDA_PORT    GPIOB
#define SDA_PIN     GPIO_Pin_11 

#define SCL_HIGH()  GPIO_SetBits(SCL_PORT, SCL_PIN)
#define SCL_LOW()   GPIO_ResetBits(SCL_PORT, SCL_PIN)
#define SDA_HIGH()  GPIO_SetBits(SDA_PORT, SDA_PIN)
#define SDA_LOW()   GPIO_ResetBits(SDA_PORT, SDA_PIN)
#define SDA_READ()  GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN)
#define DELAY()     i2c_delay()

static void i2c_delay(void)
{
    for (uint32_t n = 0; n < 14; n++)
    {
        __NOP();
    }
}

static void i2c_start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    DELAY();
    SDA_LOW();
    DELAY();
    SCL_LOW();
}

static void i2c_stop(void)
{
    SDA_LOW();
    DELAY();
    SCL_HIGH();
    DELAY();
    SDA_HIGH();
    DELAY();
}

/**
 * @brief  软件I2C写
 * @param  data
 * @return bool
 */
static bool i2c_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // MSB
        if (data & 0x80)
            SDA_HIGH();
        else
            SDA_LOW();
        DELAY();
        SCL_HIGH();
        DELAY();
        SCL_LOW();
        data <<= 1;
    }
    // 释放SDA
    SDA_HIGH();
    DELAY();
    SCL_HIGH();
    DELAY();
    // 应答位，0-ACK
    if (SDA_READ())
    {
        SCL_LOW();
        return false;
    }
    SCL_LOW();
    DELAY();
    return true;
}

static uint8_t i2c_read_byte(bool ack)
{
    uint8_t data = 0;
    // 释放SDA
    SDA_HIGH();
    for (uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        SCL_HIGH();
        DELAY();
        if (SDA_READ())
            data |= 0x01;
        SCL_LOW();
        DELAY();
    }
    if (ack)
        SDA_LOW();
    else
        SDA_HIGH();
    DELAY();
    // 等待从机读取ACK
    SCL_HIGH();
    DELAY();
    SCL_LOW();
    SDA_HIGH();
    DELAY();
    return data;
}

static void i2c_io_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SCL_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

void swi2c_init(void)
{
    i2c_io_init();
}

bool swi2c_write(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t length)
{
    i2c_start();
    if (!i2c_write_byte(addr << 1))
    {
        i2c_stop();
        return false;
    }
    i2c_write_byte(reg);
    for (uint16_t i = 0; i < length; i++)
    {
        i2c_write_byte(data[i]);
    }
    i2c_stop();
    return true;
}

bool swi2c_read(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t length)
{
    i2c_start();
    if (!i2c_write_byte(addr << 1))
    {
        i2c_stop();
        return false;
    }
    i2c_write_byte(reg);
    i2c_start();
    i2c_write_byte((addr << 1) | 0x01);
    for (uint16_t i = 0; i < length; i++)
    {
        // 最后一个字节发送NACK
        data[i] = i2c_read_byte(i == length - 1 ? false : true);
    }
    i2c_stop();
    return true;
}
