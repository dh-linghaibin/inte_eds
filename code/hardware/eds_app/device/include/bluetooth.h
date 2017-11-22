/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef enum {
	B_ENABLE = 0,
	B_CLOSE  = 1
} power_e;

typedef struct _bluetooth_obj {
	void (*init)(struct _bluetooth_obj *blue);
	int  (*power_off)(struct _bluetooth_obj *m);
	int  (*power_on)(struct _bluetooth_obj *m);
}bluetooth_obj;

void bluetooth_register(void);

#ifdef __cplusplus
}
#endif

#endif

