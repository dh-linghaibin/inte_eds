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
	int position; /* �ŷ�λ�� */
	uint8_t stall;/* ��ǰ��λ */

	void (*init)(struct _servo_obj *servo); /* ��ʼ�� */
	void (*servo_position)(struct _servo_obj *servo,uint16_t pos); /* ����λ�� */
	void (*speed_set)(struct _servo_obj *servo,int speed);			/* �����ٶ� */
	int (*get_limit)(struct _servo_obj *servo,limit_e dr);

	void (*left_limit_cb)(void(*f)(void));  /* ����λ�ص� */
	void (*right_limit_cb)(void(*f)(void)); /* ����λ�ص� */
	void (*calibration_cb)(void(*f)(void)); /* У׼�ص� */
	void (*position_cb)(void(*f)(void));	/* ��դ�ص� */

	int  (*power_off)(struct _servo_obj *m);
	int  (*power_on)(struct _servo_obj *m);
}servo_obj;

void servo_register(void);

#ifdef __cplusplus
}
#endif

#endif

