/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "Button.h"


void (*but_callback)(enum BUTTON_TYPE type); 

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rcu_config(void) {
	rcu_periph_clock_enable(RCU_GPIOB);  /* enable GPIOA clock */
}
/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void) {
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_12 | GPIO_PIN_13); /* config the GPIO as analog mode */
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void exti_config(void) {
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOB,GPIO_EVENT_PIN_12); /* connect key EXTI line to key GPIO pin */
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOB,GPIO_EVENT_PIN_13);
	exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_RISING); /* configure key EXTI line */
	exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_flag_clear(EXTI_12);
	exti_interrupt_flag_clear(EXTI_13);
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void button_init(struct _button_obj *button) {
	rcu_config();
	gpio_config();
	//exti_config();
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
int button_get(struct _button_obj *button,enum BUTTON_TYPE type) {
	if(type == ADD) {
		return gpio_input_bit_get(GPIOB,GPIO_PIN_12);
	} else {
		return gpio_input_bit_get(GPIOB,GPIO_PIN_13);
	}
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static int button_power_off(struct _button_obj *button) {
	exti_config();
	return D_OK;
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static int button_power_on(struct _button_obj *button) {
	return D_OK;
}

static void button_callback(void(*f)(enum BUTTON_TYPE type)) {
	but_callback = f;
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void button_register(void) {
	struct _button_obj *button = GET_DAV(struct _button_obj);
	
	button->init      = &button_init;
	button->callback  = &button_callback;
	button->power_off = &button_power_off;
	button->power_on  = &button_power_on;
	button->get       = &button_get;

    register_dev_obj("but",button);
}
