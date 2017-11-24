/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

enum BUTTON_TYPE{
	ADD = 0,
	SUB,
};

typedef struct _button_obj {
	void (*init)(struct _button_obj *button);
	int (*get)(struct _button_obj *button,enum BUTTON_TYPE type);
	int  (*power_off)(struct _button_obj *button);
	int  (*power_on)(struct _button_obj *button);
	void (*callback)(void(*f)(enum BUTTON_TYPE type));
}button_obj;

void button_register(void);

#ifdef __cplusplus
}
#endif

#endif
