/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef __H_
#define __H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _iwdg_obj {
	void(*init)(struct _iwdg_obj *iwdg);
	void(*reload)(struct _iwdg_obj *iwdg);
}iwdg_obj;

void iwdg_register(void);

#ifdef __cplusplus
}
#endif

#endif

