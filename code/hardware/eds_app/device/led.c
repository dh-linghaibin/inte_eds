
#include "led.h"

static void led_init(struct _led_obj *led) {
	/* enable the led clock */
    rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	
	rcu_periph_clock_enable(RCU_AF);
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);

    /* configure led GPIO port */ 
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_13);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_15);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_3);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_4);

    GPIO_BOP(GPIOC) = GPIO_PIN_13;
	GPIO_BOP(GPIOA) = GPIO_PIN_15;
	GPIO_BOP(GPIOB) = GPIO_PIN_3;
	GPIO_BOP(GPIOB) = GPIO_PIN_4;

	//printf("led_init successfully!\n\r");
}

static void led_set(struct _led_obj *led,enum LED_TYPE type,enum LED_NAME name) {
	switch(type) {
		case ON:
			GPIO_BC(led->pin[name].port) = led->pin[name].pin;
			break;
		case OFF:
			GPIO_BOP(led->pin[name].port) = led->pin[name].pin;
			break;
		case TOGGLE:
			gpio_bit_write(led->pin[name].port, led->pin[name].pin, 
			(bit_status)(1-gpio_input_bit_get(led->pin[name].port, led->pin[name].pin)));
			break;
	}
	//printf("led_set %d  %d!\n\r",type,name);
}

void led_register(void) {
    struct _led_obj *led = GET_DAV(struct _led_obj);
	
	led->pin[0].port = GPIOC;
	led->pin[0].pin = GPIO_PIN_13;
	led->pin[1].port = GPIOA;
	led->pin[1].pin = GPIO_PIN_15;
	led->pin[2].port = GPIOB;
	led->pin[2].pin = GPIO_PIN_3;
	led->pin[3].port = GPIOB;
	led->pin[3].pin = GPIO_PIN_4;

	led->init = &led_init;
	led->set = &led_set;
    register_dev_obj("led",led);
}


