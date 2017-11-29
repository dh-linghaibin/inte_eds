/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _flash_obj {
	void (*init)(struct _flash_obj *flash);
	int (*write)(struct _flash_obj *flash,uint32_t address,uint32_t data);
	int (*read)(struct _flash_obj *flash,uint32_t address,uint32_t *read_data);
} flash_obj;

#ifdef __cplusplus
}
#endif

#endif
