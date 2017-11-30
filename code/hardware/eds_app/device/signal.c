/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "signal.h"

static uint16_t bike_speed_count = 0;
static uint16_t bike_speed = 0;

static uint16_t bike_cadence_count = 0;
static uint16_t bike_cadence = 0;

/**
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
  */
static void gpio_configuration(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);

    /*configure PA6 (TIMER2 CH0) as alternate function*/
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_14);
	GPIO_BOP(GPIOB) = GPIO_PIN_14;
}

/**
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
  */
static void nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(TIMER1_IRQn, 1, 1);
	nvic_irq_enable(TIMER2_IRQn, 1, 2);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
static void timer_configuration(void)
{
    /* TIMER2 configuration: input capture mode -------------------
    the external signal is connected to TIMER2 CH0 pin (PA6)
    the rising edge is used as active edge
    the TIMER2 CH0CV is used to compute the frequency value
    ------------------------------------------------------------ */
    timer_ic_parameter_struct timer_icinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);

    /* TIMER2 configuration */
    timer_initpara.prescaler         = 107;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

    /* TIMER2  configuration */
    /* TIMER2 CH0 input capture configuration */
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;
    timer_icinitpara.icselection = TIMER_IC_POLARITY_FALLING;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x0;

    timer_input_capture_config(TIMER1,TIMER_CH_2,&timer_icinitpara);
	timer_input_capture_config(TIMER1,TIMER_CH_3,&timer_icinitpara);
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);

    /* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH2);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER1,TIMER_INT_CH2);

	/* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH3);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER1,TIMER_INT_CH3);

    /* TIMER2 counter enable */
    timer_enable(TIMER1);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void) {
    /* -----------------------------------------------------------------------
    TIMER0 configuration:
    generate 3 complementary PWM signal.
    TIMER0CLK is fixed to systemcoreclock, the TIMER0 prescaler is equal to 108 
    so the TIMER0 counter clock used is 1MHz.
    insert a dead time equal to (64 + 36) * 2 / systemcoreclock =1.85us 
    configure the break feature, active at low level, and using the automatic
    output enable feature.
    use the locking parameters level 0.
    ----------------------------------------------------------------------- */
    //timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    //timer_break_parameter_struct timer_breakpara;

    rcu_periph_clock_enable(RCU_TIMER2);

    timer_deinit(TIMER2);

    /* TIMER0 configuration */
    timer_initpara.prescaler         = 375 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 100;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2,&timer_initpara);

    /* TIMER0 channel control update interrupt enable */
    timer_interrupt_enable(TIMER2,TIMER_INT_UP);

    /* TIMER0 counter enable */
    timer_enable(TIMER2);
}

static void signal_init(struct _signal_obj *signal) {
	gpio_configuration(); 
	nvic_configuration();
	timer_configuration();
	timer_config();
}

uint16_t signal_get_speed(struct _signal_obj *signal) {
	return bike_speed;
}

uint16_t signal_get_cadence(struct _signal_obj *signal) {
	return bike_cadence;
}

static int signal_power_off(struct _signal_obj *m) {
	GPIO_BC(GPIOB) = GPIO_PIN_14;
	timer_disable(TIMER2);
	return D_OK;
}

static int signal_power_on(struct _signal_obj *m) {
	GPIO_BOP(GPIOB) = GPIO_PIN_14;
	 timer_enable(TIMER2);
	return D_OK;
}


void signal_register(void) {
	struct _signal_obj *signal = GET_DAV(struct _signal_obj);

	signal->init 	    = &signal_init;
	signal->get_speed   = &signal_get_speed;
	signal->get_cadence = &signal_get_cadence;
	signal->power_off   = &signal_power_off;
	signal->power_on    = &signal_power_on;

    register_dev_obj("sig",signal);
}

/**
  * @brief  This function handles TIMER2 interrupt request.
  * @param  None
  * @retval None
  */
void TIMER1_IRQHandler(void) {
    if(SET == timer_interrupt_flag_get(TIMER1,TIMER_INT_CH2)) {
		/*  */
		static uint8_t dr = 0;
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH2);
		if(0 == dr){
            dr = 1;
			bike_speed_count = 0;
        }else if(1 == dr){
			dr = 0;
			bike_speed = bike_speed_count;
        }
    }

	if(SET == timer_interrupt_flag_get(TIMER1,TIMER_INT_CH3)) {
		/*  */
		static uint8_t dr = 0;
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH3);
		if(0 == dr){
            dr = 1;
			bike_cadence_count = 0;
        }else if(1 == dr){
			dr = 0;
			bike_cadence = bike_cadence_count;
        }
    }
}
/*!
    \brief      this function handles TIMER0 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TIMER2_IRQHandler(void) {
    /* clear TIMER interrupt flag */
    timer_interrupt_flag_clear(TIMER2,TIMER_INT_UP);
	if(bike_speed_count < 3000) {
		bike_speed_count++;
	} else {
		bike_speed = bike_speed_count;
	}
	
	if(bike_cadence_count < 15000) {
		bike_cadence_count++;
	} else {
		bike_cadence = bike_cadence_count;
	}
}
           

