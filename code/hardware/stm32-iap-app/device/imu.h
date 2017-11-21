#ifndef _IMU_H_
#define _IMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include "mini_filter.h"
#include <math.h>

#define PI 3.1415926535f
#define math_square(num) (((float)num) * ((float)num))
#define math_abs(num) ((num) < 0 ? -(num) : (num))
#define math_limit(num, max, min) ((num) > max ? max : (num) < min ? min : (num)) //数据限幅
#define math_rad(num) ((num)*PI / 180.0f)                                         //角度转弧度
#define math_degree(num) ((num) / PI * 180.0f)                                    //弧度转角度

#define KpDef 0.8f            //0.16
#define KiDef 0.0005f         //0.001
#define SampleRateHalf 0.001f //0.001

#define Gyro_G 	0.03051756	 //  1/(65535/2000)=0.03051756   陀螺仪初始化+-1000度每秒
#define Gyro_Gr	0.0005426		//   AtR*Gyro_G

struct __Ang {
    float Pitch;
    float Roll;
    float Yaw;
};
//角度
typedef struct
{
	struct __Ang radian;//弧度值
	struct __Ang angle;//角度值
}EulerAngle;
/*  重力  */
typedef __IO struct
{
  float x;
  float y;
  float z;
}Gravity;

/* 四元数 */
typedef __IO struct {
    float q0;
    float q1;
    float q2;
    float q3;
} Quaternion;

struct _int16
{
    int16_t x;
    int16_t y;
    int16_t z;
};

struct _float
{
    float x;
    float y;
    float z;
};

typedef struct
{
    struct _int16 origin; //原始值
    struct _float averag; //平均值
    struct _float histor; //历史值
    struct _int16 quiet;  //静态值
    struct _float radian; //弧度值
} _sensor_data;
/*  MPU6050  */
typedef struct
{
    _sensor_data acc;
    _sensor_data gyr;
    int temp[5];
} _sensor_data_mpu;


#ifdef __cplusplus
}
#endif

#endif
