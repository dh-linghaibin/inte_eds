#ifndef _LED_H_
#define _LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "l_config.h"

enum LED_TYPE{
	ON,
	OFF,
	TOGGLE,
};

enum LED_NAME{
	SIGNAL = 0,
	R = 1,
	G = 2,
	B = 3,
};

struct _led_pin {
	uint32_t port;
	uint32_t pin;
};

typedef struct _led_obj {
	struct _led_pin pin[4];
	void (*init)(struct _led_obj *led);
	void (*set)(struct _led_obj *led,enum LED_TYPE type,enum LED_NAME name);
}led_obj;

void led_register(void);

#ifdef __cplusplus
}
#endif

#endif
