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
#define DELAY()     I2C_Delay()

static void I2C_Delay(void)
{
    for (uint32_t n = 0; n < 14; n++)
    {
        __NOP();
    }
}

static void I2C_Start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    DELAY();
    SDA_LOW();
    DELAY();
    SCL_LOW();
}

static void I2C_Stop(void)
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
static bool I2C_WriteByte(uint8_t data)
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

static uint8_t I2C_ReadByte(bool ack)
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

static void I2C_IO_Init(void)
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

void SWI2C_Init(void)
{
    I2C_IO_Init();
}

bool SWI2C_Write(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t length)
{
    I2C_Start();
    if (!I2C_WriteByte(addr << 1))
    {
        I2C_Stop();
        return false;
    }
    I2C_WriteByte(reg);
    for (uint16_t i = 0; i < length; i++)
    {
        I2C_WriteByte(data[i]);
    }
    I2C_Stop();
    return true;
}

bool SWI2C_Read(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t length)
{
    I2C_Start();
    if (!I2C_WriteByte(addr << 1))
    {
        I2C_Stop();
        return false;
    }
    I2C_WriteByte(reg);
    I2C_Start();
    I2C_WriteByte((addr << 1) | 0x01);
    for (uint16_t i = 0; i < length; i++)
    {
        // 最后一个字节发送NACK
        data[i] = I2C_ReadByte(i == length - 1 ? false : true);
    }
    I2C_Stop();
    return true;
}
