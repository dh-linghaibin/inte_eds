
#include "signal.h"

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

static void signal_init(struct _signal_obj *signal) {
	gpio_configuration(); 
	nvic_configuration();
	timer_configuration();
}

static int signal_power_off(struct _signal_obj *m) {
	GPIO_BC(GPIOB) = GPIO_PIN_14;
	return D_OK;
}

static int signal_power_on(struct _signal_obj *m) {
	GPIO_BOP(GPIOB) = GPIO_PIN_14;
	return D_OK;
}


void signal_register(void) {
	struct _signal_obj *signal = GET_DAV(struct _signal_obj);

	signal->init 	  = &signal_init;
	signal->power_off = &signal_power_off;
	signal->power_on  = &signal_power_on;

    register_dev_obj("sig",signal);
}

static uint32_t Rc_Pwm_In = 0;
timer_ic_parameter_struct timer_icinitpara;

__IO uint16_t readvalue1 = 0, readvalue2 = 0;
__IO uint16_t ccnumber = 0;
__IO uint32_t count = 0;
__IO float fre = 0;

/**
  * @brief  This function handles TIMER2 interrupt request.
  * @param  None
  * @retval None
  */
void TIMER1_IRQHandler(void) {
    if(SET == timer_interrupt_flag_get(TIMER1,TIMER_INT_CH2)) {
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH2);
		if(0 == ccnumber){
            /* read channel 0 capture value */
            readvalue1 = timer_channel_capture_value_register_read(TIMER1,TIMER_CH_2);
            ccnumber = 1;
        }else if(1 == ccnumber){
            /* read channel 0 capture value */
            readvalue2 = timer_channel_capture_value_register_read(TIMER1,TIMER_CH_2);

            if(readvalue2 > readvalue1){
                count = (readvalue2 - readvalue1); 
            }else{
                count = ((0xFFFF - readvalue1) + readvalue2); 
            }
            fre = (float)1000000 / count;
            ccnumber = 0;
        }
    }

	if(SET == timer_interrupt_flag_get(TIMER1,TIMER_INT_CH3)) {
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER1,TIMER_INT_CH3);
    }
}
