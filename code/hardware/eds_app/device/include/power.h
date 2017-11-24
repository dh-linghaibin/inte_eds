/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _POWER_H_
#define _POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _power_obj {
	void     (*init)(struct _power_obj * power);
	uint16_t (*get_battery)(struct _power_obj * power);
	uint16_t (*get_moto_current)(struct _power_obj * power);
	int      (*power_off)(struct _power_obj *power);
	int      (*power_on)(struct _power_obj *power);
} power_obj;

void power_register(void);

#ifdef __cplusplus
}
#endif

#endif
