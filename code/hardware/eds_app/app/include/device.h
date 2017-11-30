/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth.h"
#include "led.h"
#include "servo.h"
#include "signal.h"
#include "power.h"
#include "Button.h"
#include "rtc.h"
#include "l_math.h"
#include "iwdg.h"
#include "mpu6050_dmp.h"
#include "s_delay.h"
#include "flash.h"

void device_init(void);

#ifdef __cplusplus
}
#endif

#endif

