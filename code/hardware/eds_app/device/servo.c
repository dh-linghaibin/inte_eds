/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "servo.h"

static void(*left_limit_cb)(void);  /* 左限位回调 */
static void(*right_limit_cb)(void); /* 右限位回调 */
static void(*calibration_cb)(void); /* 校准回调 */
static void(*position_cb)(void);	 /* 光栅回调 */
extern void (*but_callback)(enum BUTTON_TYPE type);/* 按钮回调 */

static const uint16_t stall_position[11] = {0,200,400,600,800,1000,1200,1400,1600,1800,2000};
static int posstion_num = 0;			/* 电机位置 */
static uint8_t moto_dr = 0;				/* 电机方向 */
static uint16_t posstion_num_error = 0; /* 误差修正 */
static uint8_t limit_flag = 0;			/* 限位标志 */
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

	nvic_irq_enable(EXTI0_IRQn, 1U, 2U);
	nvic_irq_enable(EXTI1_IRQn, 1U, 1U);
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
	//	/* connect key EXTI line to key GPIO pin */
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOC,GPIO_EVENT_PIN_14);
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOA,GPIO_EVENT_PIN_12);
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOA,GPIO_EVENT_PIN_0);
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOA,GPIO_EVENT_PIN_1);
	//	/* configure key EXTI line */
	exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
	exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_init(EXTI_14, EXTI_INTERRUPT, EXTI_TRIG_RISING);

	exti_interrupt_flag_clear(EXTI_0);
	exti_interrupt_flag_clear(EXTI_1);
	exti_interrupt_flag_clear(EXTI_12);
	exti_interrupt_flag_clear(EXTI_14);
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

	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0);  		/* CH3 configuration in PWM mode1,duty cycle 75% */
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
	GPIO_BOP(GPIOA) = GPIO_PIN_8; /*设备打开*/

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
/**
    \brief      servo_power_on
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static void servo_moto_set_speed(struct _servo_obj *servo,int speed) {
	if(speed == 0) {
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0); 
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0);
	} else if(speed > 0) {
		moto_dr = 0;
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0); 
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,speed); 
	} else {
		moto_dr = 1;
		speed = 0-speed;
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,speed); 
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0); 
	}	
}
/**
    \brief      servo_get_limit
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static int servo_get_limit(struct _servo_obj *servo,limit_e dr) {
	if(dr == RIGHT) {
		return gpio_input_bit_get(GPIOA,GPIO_PIN_12);
	} else {
		return gpio_input_bit_get(GPIOC,GPIO_PIN_14);
	}
}
/**
    \brief      servo_get_limit
    \param[in]  struct _servo_obj
    \param[out] none
    \retval     none
*/
static int servo_to_stall(servo_obj  *servo,power_obj *power,stall_e stall) {
	uint8_t next_stall = 0;
	switch(stall) {
		case S_ADD:{
			if(servo->stall < 10) { /* 档位只有十档 */
				next_stall = servo->stall+1;
			} else {
				return 0;
			}
		} break;
		case S_SUB:{
				if(servo->stall > 0) {
					next_stall = servo->stall-1;
				} else {
					return 0;
				}
		}break;
	}
	limit_flag = 0; /* 重新检测电机位置 */
	if(stall_position[next_stall] > posstion_num) {
		uint16_t to_setp = stall_position[next_stall] - posstion_num; 
		if(to_setp > 10) {
			if(servo->get_limit(servo,RIGHT) == 0) { /* 位置保护 */
				servo->speed_set(servo,1000);
				uint8_t l_breaks = 0;	 	/* 提前刹车值 */
				uint16_t last_power = 0;	/* 功率缓冲值 */
				uint32_t time_out = 0;	  	/* 超时保护时间 */
				uint16_t current_count = 0; /* 电流保护 */
				while(posstion_num < (stall_position[next_stall]) - l_breaks) {			 /* 计算刹车量 */	
					last_power = power->get_moto_current(power) * power->get_battery(power) / 1000; /* 计算功率 */
					/* 刹车值计算 防止多走 */
					if( ( (stall_position[next_stall] - posstion_num) < 70 ) && 
									( (last_power < 310) && (last_power > 135) ) ) {
						//l_breaks = (uint8_t)( (310-last_power) * 0.395);
						//printf("%d \r\n",l_breaks);
					}
					/* 过流保护 */
					if(last_power > 390) {
						if(current_count > 5000) {
							servo->speed_set(servo,0);
							return 1;
						} else {
							current_count++;
						}
					} else {
						current_count = 0;
					}
					/* 电机碰到限位 */
					if(limit_flag == 1) {
						servo->speed_set(servo,0);
						return 3;
					}
					/* 超时计算 */
					if(time_out < 700000) {
						time_out++;
					} else {
						time_out = 0;
						servo->speed_set(servo,0);
						return 2;
					}
				}
				servo->speed_set(servo,0);
			} else {
				return 3;
			}
		}
	} else {
		uint16_t to_setp = posstion_num - stall_position[next_stall];
		if(to_setp > 10) {
			if(servo->get_limit(servo,LEFT) == 0) { /* 位置保护 */
				servo->speed_set(servo,-1000);
				uint8_t l_breaks = 0;	 	/* 提前刹车值 */
				uint16_t last_power = 0;	/* 功率缓冲值 */
				uint32_t time_out = 0;	  	/* 超时保护时间 */
				uint16_t current_count = 0; /* 电流保护 */
				while(posstion_num > (stall_position[next_stall] + l_breaks)) {
					last_power = power->get_moto_current(power) * power->get_battery(power) / 1000; /* 计算功率 */
					/* 刹车值计算 防止多走 */
					if( ( (stall_position[next_stall] - posstion_num) < 70 ) && 
									( (last_power < 310) && (last_power > 135) ) ) {
						//l_breaks = (uint8_t)( (310-last_power) * 0.395);
						//printf("%d \r\n",l_breaks);
					}
					/* 过流保护 */
					if(last_power > 390) {
						if(current_count > 5000) {
							servo->speed_set(servo,0);
							return 1;
						} else {
							current_count++;
						}
					} else {
						current_count = 0;
					}

					/* 电机碰到限位 */
					if(limit_flag == 1) {
						servo->speed_set(servo,0);
						return 3;
					}
				
					/* 超时计算 */
					if(time_out < 700000) {
						time_out++;
					} else {
						time_out = 0;
						servo->speed_set(servo,0);
						return 2;
					}
				}
				servo->speed_set(servo,0);
			} else {
				return 3;
			}
		}
	}
	servo->stall = next_stall; /* 成功 设备档位 */
	return 0;
}
/**
    \brief      servo_left_limit_cb
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
static void servo_left_limit_cb(void(*f)(void)) {
	left_limit_cb = f;
}
/**
    \brief      servo_left_limit_cb
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
static void servo_right_limit_cb(void(*f)(void)) {
	right_limit_cb = f;
}
/**
    \brief      servo_left_limit_cb
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
static void servo_calibration_cb(void(*f)(void)) {
	calibration_cb = f;
}
/**
    \brief      servo_left_limit_cb
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
static void servo_position_cb(void(*f)(void)) {
	position_cb = f;
}
/**
    \brief      servo_left_limit_cb
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
int servo_get_posstion(struct _servo_obj *servo){
	return posstion_num;
}
/**
    \brief      servo_set_posstion
    \param[in]  void(*f)(void)
    \param[out] none
    \retval     none
*/
void servo_set_posstion(struct _servo_obj *servo,int posstion){
	posstion_num = posstion;
}
/**
    \brief      servo_register
    \param[in]  none
    \param[out] none
    \retval     none
*/
void servo_register(void) {
	struct _servo_obj *servo = GET_DAV(struct _servo_obj);
	
	servo->init           = &servo_init;
	servo->power_off      = &servo_power_off;
	servo->power_on       = &servo_power_on;
	servo->calibration_cb = &servo_calibration_cb;
	servo->left_limit_cb  = &servo_left_limit_cb;
	servo->position_cb    = &servo_position_cb;
	servo->right_limit_cb = &servo_right_limit_cb;
	servo->speed_set      = &servo_moto_set_speed;
	servo->get_limit	  = &servo_get_limit;
	servo->to_stall		  = &servo_to_stall;
	servo->get_posstion   = &servo_get_posstion;
	servo->set_posstion   = &servo_set_posstion;

    register_dev_obj("ser",servo);
}

void EXTI0_IRQHandler(void) {
	if (RESET != exti_interrupt_flag_get(EXTI_0)) {
		exti_interrupt_flag_clear(EXTI_0);
		posstion_num = 500; /* 电机位置校准 */
		if(calibration_cb != 0) {
			calibration_cb();
		}
	}
}


void EXTI1_IRQHandler(void) {
	if (RESET != exti_interrupt_flag_get(EXTI_1)) {
		exti_interrupt_flag_clear(EXTI_1);
		if(moto_dr == 0) {
			posstion_num++;
			if(posstion_num_error >= 70) {
				posstion_num_error = 0;
				posstion_num -= 1;
			} else {
				posstion_num_error++;
			}
		} else {
			posstion_num--;
		}
		if(position_cb != 0) {
			position_cb();
		}
	}
}

void EXTI10_15_IRQHandler(void) {
    if (RESET != exti_interrupt_flag_get(EXTI_14)) {
        exti_interrupt_flag_clear(EXTI_14);
		/* 关闭电机 限位位置 电机不允许运行 */
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0); 
		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0); 
		limit_flag = 1; /* 通知电机运动程序 */
		if(left_limit_cb != 0) {
			left_limit_cb();
		}
    } else if(RESET != exti_interrupt_flag_get(EXTI_12)) {
		exti_interrupt_flag_clear(EXTI_12);
		if(gpio_input_bit_get(GPIOA,GPIO_PIN_12) == SET) {
			/* 关闭电机 限位位置 电机不允许运行 */
			timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0); 
			timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0); 
			limit_flag = 1; /* 通知电机运动程序 */
			if(right_limit_cb != 0) {
				right_limit_cb();
			}
		}
		if(gpio_input_bit_get(GPIOB,GPIO_PIN_12) == SET) {
			if(but_callback != 0) {
				but_callback(B_ADD);
			}
		}
	} else if(RESET != exti_interrupt_flag_get(EXTI_13)) {
		exti_interrupt_flag_clear(EXTI_13);
		if(but_callback != 0) {
			but_callback(B_SUB);
		}
	}
}

