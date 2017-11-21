

#ifndef _SERVO_H_
#define _SERVO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

typedef struct _servo_cb_obj {
	void (*left_limit_cb)(void(*f)(void));  /* 左限位回调 */
	void (*right_limit_cb)(void(*f)(void)); /* 右限位回调 */
	void (*calibration_cb)(void(*f)(void)); /* 校准回调 */
	void (*position_cb)(void(*f)(void));	 /* 光栅回调 */
}servo_cb_obj;

typedef struct _servo_obj {
	int position; /* 伺服位置 */

	void (*init)(struct _servo_obj *servo);
	void (*servo_position)(struct _servo_obj *servo,uint16_t pos);
	int  (*power_off)(struct _servo_obj *m);
	int  (*power_on)(struct _servo_obj *m);
	servo_cb_obj *even_cb;
}servo_obj;

void servo_register(void);

#ifdef __cplusplus
}
#endif

#endif

