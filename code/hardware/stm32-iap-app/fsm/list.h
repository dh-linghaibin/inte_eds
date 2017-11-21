
#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _device {
    void(*init)(void);/*初始化设备*/
    void(*write)(unsigned char type,...);/*写设备*/
    void(*read)(unsigned char type,...);/*读设备*/
	void(*control)(unsigned char type,...);/*设备控制*/
}device;

typedef struct _doublelist{
	char name[10];/*设备名称*/   
	struct _doublelist *next,*down;
	device *dev;/*设备*/
}doublelist;

int list_test(void);

#ifdef __cplusplus
}
#endif

#endif
