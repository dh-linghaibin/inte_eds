/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _signal_obj {
	void (*init)(struct _signal_obj *signal);
	uint16_t (*get_speed)(struct _signal_obj *signal);
	uint16_t (*get_cadence)(struct _signal_obj *signal);
	int  (*power_off)(struct _signal_obj *m);
	int  (*power_on)(struct _signal_obj *m);
}signal_obj;

void signal_register(void);

#ifdef __cplusplus
}
#endif

#endif
