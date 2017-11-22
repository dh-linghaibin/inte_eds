/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _RTC_H_
#define _RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _rtc_obj {
	void(*init)(struct _rtc_obj* rtc);
	void(*callback)(void (*f)(void));
}rtc_obj;

void rtc_register(void);

#ifdef __cplusplus
}
#endif

#endif
