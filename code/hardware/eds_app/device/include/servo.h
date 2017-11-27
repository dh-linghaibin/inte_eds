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

typedef enum {
	RIGHT,
	LEFT
}limit_e;

typedef enum {
	S_ADD = 0,
	S_SUB
}stall_e;

typedef struct _servo_obj {
	int position; /* 伺服位置 */
	uint8_t stall;/* 当前档位 */

	void (*init)(struct _servo_obj *servo); /* 初始化 */
	void (*servo_position)(struct _servo_obj *servo,uint16_t pos); /* 设置位置 */
	void (*speed_set)(struct _servo_obj *servo,int speed);			/* 设置速度 */
	int (*get_limit)(struct _servo_obj *servo,limit_e dr);

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

