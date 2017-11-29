/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _SERVO_H_
#define _SERVO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"
#include "power.h"
#include "Button.h"

typedef enum {
	RIGHT,
	LEFT
}limit_e;

typedef enum {
	S_ADD = 0,
	S_SUB
}stall_e;

typedef struct _servo_obj {
	uint8_t stall;/* 当前档位 */

	void (*init)(struct _servo_obj *servo); 									/* 初始化 */
	int  (*to_stall)(struct _servo_obj  *servo,power_obj *power,stall_e stall); /* 设置位置 */
	void (*speed_set)(struct _servo_obj *servo,int speed);					    /* 设置速度 */
	int  (*get_limit)(struct _servo_obj *servo,limit_e dr);					    /* 获取限位状态 */
	int  (*get_posstion)(struct _servo_obj *servo);							    /* 获取当前位置 */
	void (*set_posstion)(struct _servo_obj *servo,int posstion);			    /* 设置当前位置 */

	void (*left_limit_cb)(void(*f)(void));  /* 左限位回调 */
	void (*right_limit_cb)(void(*f)(void)); /* 右限位回调 */
	void (*calibration_cb)(void(*f)(void)); /* 校准回调 */
	void (*position_cb)(void(*f)(void));	/* 光栅回调 */

	int  (*power_off)(struct _servo_obj *m);
	int  (*power_on)(struct _servo_obj *m);
}servo_obj;

void servo_register(void);

#ifdef __cplusplus
}
#endif

#endif

