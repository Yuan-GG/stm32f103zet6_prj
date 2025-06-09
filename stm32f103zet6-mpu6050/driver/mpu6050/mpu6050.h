#ifndef __MPU6050_H__
#define __MPU6050_H__


#include <stdbool.h>
#include <stdint.h>


typedef struct{
    float x;
    float y;
    float z;
} mpu6050_accel_t;

typedef struct{
    float x;
    float y;
    float z;
} mpu6050_gyro_t;

bool MPU6050_Init(void);
bool MPU6050_Read_Accel(mpu6050_accel_t *accel);
bool MPU6050_Read_Gyro(mpu6050_gyro_t *gyro);
bool MPU6050_Read_Temper(float *temper);


#endif /* __MPU6050_H__ */
