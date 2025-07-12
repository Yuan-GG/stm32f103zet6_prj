#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "main.h"
#include "swi2c.h"
#include "mpu6050.h"


#define WHO_AM_I_REG        0x75
#define PWR_MGMT_1_REG      0x6B
#define SMPLRT_DIV_REG      0x19
#define ACCEL_CONFIG_REG    0x1C
#define ACCEL_XOUT_H_REG    0x3B
#define TEMP_OUT_H_REG      0x41
#define GYRO_CONFIG_REG     0x1B
#define GYRO_XOUT_H_REG     0x43

#define MPU6050_ADDR    0x68

/**
 * @brief  MPU6050初始化
 * @param  None
 * @return None
 */
bool MPU6050_Init(void){
    swi2c_init();
    // read whoami
    uint8_t whoami;
    swi2c_read(MPU6050_ADDR, WHO_AM_I_REG, &whoami, 1);
    if (whoami != MPU6050_ADDR){
        return false;
    }

    uint8_t data;
    
    // reset the device
    data = 0x80;
    swi2c_write(MPU6050_ADDR, PWR_MGMT_1_REG, &data, 1);
    delayms(10);

    // set the sample rate to 1kHz
    data = 0x07;
    swi2c_write(MPU6050_ADDR, SMPLRT_DIV_REG, &data, 1);

    // set the accelerometer scale range to ±2g
    data = 0x00;
    swi2c_write(MPU6050_ADDR, ACCEL_CONFIG_REG, &data, 1);

    // set the gyroscope scale range to ±250°/s
    data = 0x00;
    swi2c_write(MPU6050_ADDR, GYRO_CONFIG_REG, &data, 1);

    return true;
}

/**
 * @brief  读取MPU6050加速度轴  
 * @param  accel  加速度结构体
 * @return None
 */
bool MPU6050_Read_Accel(mpu6050_accel_t *accel){
    bool ret;
    uint8_t data[6];

    ret = swi2c_read(MPU6050_ADDR, ACCEL_XOUT_H_REG, data, 6);
    if(!ret)
        return false;

    // 精度由ACCEL_CONFIG_REG的设置的值决定，也就是65536 / data
    accel->x = (int16_t)(data[0] << 8 | data[1]) / 16384.0f;
    accel->y = (int16_t)(data[2] << 8 | data[3]) / 16384.0f;
    accel->z = (int16_t)(data[4] << 8 | data[5]) / 16384.0f;
    return true;
}

bool MPU6050_Read_Gyro(mpu6050_gyro_t *gyro){
    bool ret;
    uint8_t data[6];

    ret = swi2c_read(MPU6050_ADDR, GYRO_XOUT_H_REG, data, 6);
    if(!ret)
        return false;

    // 精度由GYRO_CONFIG_REG的设置的值决定，也就是65536 / data
    gyro->x = (int16_t)(data[0] << 8 | data[1]) / 131.0f;
    gyro->y = (int16_t)(data[2] << 8 | data[3]) / 131.0f;
    gyro->z = (int16_t)(data[4] << 8 | data[5]) / 131.0f;
    return true;
}
bool MPU6050_Read_Temper(float *temper){
    bool ret;
    uint8_t data[2];

    ret = swi2c_read(MPU6050_ADDR, TEMP_OUT_H_REG, data, 2);
    if(!ret)
        return false;
    
    *temper = (int16_t)(data[0] << 8 | data[1]) / 340.0f + 36.53f;
    return true;
}
