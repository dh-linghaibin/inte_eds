/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _DEV_OBJ_H_
#define _DEV_OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
/*内存支持*/
#include "sram.h"

typedef enum {
	D_OK = 0,
	D_ERROR = -1
}dev_state_e;

//typedef struct _gpio_obj {
//	int num;
//}gpio_obj;

//typedef struct _pwm {
//	int num;
//}pwm;

//typedef struct _adc_obj {
//	int num;
//}adc_obj;

//typedef struct _iic_obj {
//	int num;
//}iic_obj;

//typedef struct _iic_flash {
//	int num;
//}flash_obj;

typedef struct _dev_obj {
    char name[10];/*设备名称*/   
	struct _dev_obj *next,*down;
    void *dev;/*设备*/
}dev_obj;

#define GET_DAV(dev) (dev *)SramMalloc(sizeof(dev))

/*注册设备*/
int register_dev_obj(const char *name,void *dev);
/*注销设备*/
int uregister_dev_obj(const char * name);
/*获取设备*/
void * get_device(const char *name);

#ifdef __cplusplus
}
#endif

#endif
