
#ifndef _DEV_OBJ_H_
#define _DEV_OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdio.h>
/*内存支持*/
#include "sram.h"


typedef struct _device {
    void(*init)(void);/*初始化设备*/
    void(*write)(unsigned char type,...);/*写设备*/
    void(*read)(unsigned char type,...);/*读设备*/
	void(*control)(unsigned char type,...);/*设备控制*/
}device;

typedef struct _dev_obj {
    char name[10];/*设备名称*/   
	struct _dev_obj *next,*down;
    device *dev;/*设备*/
}dev_obj;

#define GET_DAV (device *)SramMalloc(sizeof(device))

/*注册设备*/
int register_dev_obj(const char *name,device *dev);
/*注销设备*/
int uregister_dev_obj(const char * name);
/*获取设备*/
device* get_device(const char *name);

#ifdef __cplusplus
}
#endif

#endif
