/*伺服系统虚拟化*/
#include "servo.h"

static void(*left_limit_cb)(void);  /* 左限位回调 */
static void(*right_limit_cb)(void); /* 右限位回调 */
static void(*calibration_cb)(void); /* 校准回调 */
static void(*position_cb)(void);	 /* 光栅回调 */

static const uint16_t stall[11] = {2000,2500,3000,3500,4000,4500,5000,5500,6000,6600,7000};
/**
    \brief      configure servo_signal_init
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void servo_signal_init(void) {
	gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_14);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_12);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_0);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_1);
}

/**
    \brief      configure servo_pwm_init
    \param[in]  none
    \param[out] none
    \retval     none
	IMER1CLK = SystemCoreClock / 108 = 1MHz
	TIMER1 channel1 duty cycle = (4000/ 16000)* 100  = 25%
*/
static void servo_pwm_init(void) {
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_TIMER0);

	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10);

	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;

	timer_deinit(TIMER0);

	timer_initpara.prescaler         = 25;	 				/* TIMER1 configuration */
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = 999;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER0,&timer_initpara);

	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH; /* CH1,CH2 and CH3 configuration in PWM mode1 */
	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

	timer_channel_output_config(TIMER0,TIMER_CH_1,&timer_ocintpara);
	timer_channel_output_config(TIMER0,TIMER_CH_2,&timer_ocintpara);

	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0); 			/* CH2 configuration in PWM mode1,duty cycle 50% */
	timer_channel_output_mode_config(TIMER0,TIMER_CH_1,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,1000);  		/* CH3 configuration in PWM mode1,duty cycle 75% */
	timer_channel_output_mode_config(TIMER0,TIMER_CH_2,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);

	timer_primary_output_config(TIMER0,ENABLE); /* TIMER0 primary output enable */
	timer_auto_reload_shadow_enable(TIMER0); 	/* auto-reload preload enable */
	timer_enable(TIMER0); 						/* auto-reload preload enable */
}

/**
    \brief      servo_init
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static void servo_init(struct _servo_obj *servo) {
	rcu_periph_clock_enable(RCU_GPIOA); /* 时钟初始化 */
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOB);

	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_6);/*电源使能 PA6*/
	GPIO_BC(GPIOA) = GPIO_PIN_6; /* 打开 */
	
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_8);
	GPIO_BC(GPIOA) = GPIO_PIN_8; /*设备打开*/

	servo_signal_init(); /*限位前 pc14 限位后 pa12 校准 pa0 转速 pa1*/
	servo_pwm_init();    /*电机sleep PA8 高打开设备 IN1 PA9 IN2 PA10  电流	PB1 模拟*/
}

/**
    \brief      servo_power_off
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static int servo_power_off(struct _servo_obj *m) {
	/* enable and set key EXTI interrupt to the lowest priority */
	nvic_irq_disable(EXTI0_IRQn);
	nvic_irq_disable(EXTI1_IRQn); /*优先级最高*/
	nvic_irq_disable(EXTI10_15_IRQn);

	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_8);
	GPIO_BC(GPIOA) = GPIO_PIN_8; /*设备休眠*/
		
	GPIO_BOP(GPIOA) = GPIO_PIN_6; /* 关闭检测电源 */
	GPIO_BC(GPIOA) = GPIO_PIN_8;  /* 关闭电机 */

	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ,GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ,GPIO_PIN_10);

	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ,GPIO_PIN_1);

	rcu_periph_clock_disable(RCU_TIMER0);
	timer_auto_reload_shadow_disable(TIMER0);
	timer_disable(TIMER0);
	
	return D_OK;
}

/**
    \brief      servo_power_on
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static int servo_power_on(struct _servo_obj *m) {
	
	return D_OK;
}

static void servo_servo_position(struct _servo_obj *servo,uint16_t pos) {
	
}

/**
    \brief      servo_register
    \param[in]  none
    \param[out] none
    \retval     none
*/
void servo_register(void) {
	struct _servo_obj *servo = GET_DAV(struct _servo_obj);
	
	servo->init      = &servo_init;
	servo->power_off = &servo_power_off;
	servo->power_on  = &servo_power_on;
	
    register_dev_obj("ser",servo);
}

void EXTI0_IRQHandler(void) {
	if (RESET != exti_interrupt_flag_get(EXTI_0)) {
		exti_interrupt_flag_clear(EXTI_0);
		if(calibration_cb != 0) {
			calibration_cb();
		}
	}
}

void EXTI1_IRQHandler(void) {
	if (RESET != exti_interrupt_flag_get(EXTI_1)) {
		exti_interrupt_flag_clear(EXTI_1);
		if(position_cb != 0) {
			position_cb();
		}
	}
}

void EXTI10_15_IRQHandler(void) {
    if (RESET != exti_interrupt_flag_get(EXTI_14)) {
        exti_interrupt_flag_clear(EXTI_14);
		if(left_limit_cb != 0) {
			left_limit_cb();
		}
    } else if(RESET != exti_interrupt_flag_get(EXTI_12)) {
		exti_interrupt_flag_clear(EXTI_12);
		if(right_limit_cb != 0) {
			right_limit_cb();
		}
	}
}

