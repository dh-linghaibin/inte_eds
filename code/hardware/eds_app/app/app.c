/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include <stdio.h>
#include "bluetooth.h"
#include "led.h"
#include "servo.h"
#include "signal.h"
#include "mpu6050_dmp.h"
#include "power.h"
#include "Button.h"
#include "rtc.h"
#include "l_math.h"
#include "iwdg.h"
#include "s_delay.h"
#include "fsm.h"

#include "device.h"

/*设置中断向量*/
void app_set(void) {
	#define IAP_FLASH_SIZE  0x3000
	#define ApplicationAddress  0x8003000
	nvic_vector_table_set(ApplicationAddress,IAP_FLASH_SIZE);
}

void systick_config(void) {
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

static uint8_t run_1ms_flag = 0;
void SysTick_Handler(void) {
	run_1ms_flag++;
}

/*
 * task for led
 */
simple_fsm(LedTask,
	led_obj        *led;	
	power_obj      *power;
	mpu6050dmp_obj *mpu6050;
	signal_obj *signal;		)
fsm_init_name(LedTask)
	me.led = get_device("led");	
	if(me.led == NULL) {
		fsm_task_off(LedTask); /* get led faild out the task */
		return 0;
	}
	me.power = get_device("pow");
	if(me.power == NULL) {
		fsm_task_off(LedTask); /* get power faild out the task */
		return 0;
	}
	me.mpu6050 = get_device("mpu");
	if(me.mpu6050 == NULL) {
		fsm_task_off(LedTask); /* get mpu6050 faild out the task */
		return 0;
	}
	me.signal = get_device("sig");
	if(me.signal == NULL) {
		fsm_task_off(LedTask); /* get signal faild out the task */
		return 0;
	}
	while(1) {
		WaitX(250);  
		me.led->set(me.led,TOGGLE,R);
		me.mpu6050->get_pry(me.mpu6050);
		printf("mpu : %f2    %f2    %f2  speed:%d  cadence:%d \r\n",me.mpu6050->yaw,me.mpu6050->pitch,me.mpu6050->roll,me.signal->get_speed(me.signal),me.signal->get_cadence(me.signal));
//		WaitX(250);  
//		printf("bettery: %d V \r\n",me.power->get_moto_current(me.power));
	}
fsm_end



int posstion_num = 0;
uint8_t moto_dr = 0;
void position_cb(void) {
	if(moto_dr == 0) {
		posstion_num++;
	} else {
		posstion_num--;
	}
}

void calibration_cb(void) {
	//posstion_num = 0;
}

void right_limit_cb(void) {
	servo_obj  *servo = get_device("ser");
	if(servo == NULL) {
		servo->speed_set(servo,0);
	}
}

void left_limit_cb(void) {
	servo_obj  *servo = get_device("ser");
	if(servo == NULL) {
		servo->speed_set(servo,0);
	}
}


static const uint16_t stall_position[11] = {0,200,400,600,800,1000,1200,1400,1600,1800,2000};

int servo_step(servo_obj  *servo,power_obj *power,stall_e stall) {
	switch(stall) {
		case S_ADD:{
			if(servo->stall < 10) { /* 档位只有十档 */
				servo->stall++;
			}
			
		} break;
		case S_SUB:{
				if(servo->stall > 0) {
					servo->stall--;
				}
		}break;
	}
	if(stall_position[servo->stall] > posstion_num) {
		uint16_t to_setp = stall_position[servo->stall] - posstion_num;
		if(to_setp > 10) {
			moto_dr = 0;
			servo->speed_set(servo,1000);
			uint8_t breaks = 0;
			uint16_t last = 0;
			while(posstion_num <= (stall_position[servo->stall])) {
				last = power->get_moto_current(power);
				if(last > 280) {

				} else if(last > 280) {
					breaks = 5;
				} else if(last > 250) {
					breaks = 10;
				} else if(last > 210) {
					breaks = 20;
				} else if(last > 180) {
					breaks = 30;
				} else {
					breaks = 35;
				}
			}
			servo->speed_set(servo,0);
		} else {
			return 1;
		}
	} else {
		uint16_t to_setp = posstion_num - stall_position[servo->stall];
		if(to_setp > 10) {
			moto_dr = 0;
			servo->speed_set(servo,-1000);
			uint8_t breaks = 0;
			uint16_t last = 0;
			while(posstion_num >= (stall_position[servo->stall])) {
				last = power->get_moto_current(power);
				if(last > 280) {

				} else if(last > 280) {
					breaks = 5;
				} else if(last > 250) {
					breaks = 10;
				} else if(last > 210) {
					breaks = 20;
				} else if(last > 180) {
					breaks = 30;
				} else {
					breaks = 35;
				}
			}
			servo->speed_set(servo,0);
		} else {
			return 1;
		}
	}
	return 0;
}

uint16_t last;
#define erroe_num 200
static uint16_t rotate_num_error = 0;
static uint16_t rotate_num_error2 = 0; /* 二次系数累加 提高精度 */

#define but 0

simple_fsm(moto,
	uint8_t    but_sub_count;
	uint8_t    but_add_count;
	servo_obj  *servo;
	button_obj *button; 
	power_obj *power; 		)

fsm_init_name(moto)
	me.servo = get_device("ser");
	if(me.servo == NULL) {
		fsm_task_off(moto); /* get servo faild out the task */
		return 0;
	}
	me.button = get_device("but");	
	if(me.button == NULL) {
		fsm_task_off(moto); /* get button faild out the task */
		return 0;
	}
	me.power = get_device("pow");
	if(me.power == NULL) {
		fsm_task_off(LedTask); /* get power faild out the task */
		return 0;
	}
	me.servo->position_cb(position_cb);
	me.servo->calibration_cb(calibration_cb);
	me.servo->right_limit_cb(right_limit_cb);
	me.servo->left_limit_cb(left_limit_cb);
	while(1) {

		WaitX(1000);

		uint16_t step = 600;
		uint16_t before = posstion_num;
	
		moto_dr = 0;
		me.servo->speed_set(me.servo,1000);
		uint8_t breaks = 0;
		while(posstion_num < (step)) {
			//WaitX(0);
			last = me.power->get_moto_current(me.power);
			if(last > 280) {

			} else if(last > 280) {
				breaks = 5;
			} else if(last > 250) {
				breaks = 10;
			} else if(last > 210) {
				breaks = 20;
			} else if(last > 180) {
				breaks = 30;
			} else {
				breaks = 35;
			}
		}
		me.servo->speed_set(me.servo,0);
		/* 走完之后再计算误差 */
		uint16_t after = posstion_num - before;
	
		uint16_t wc = after+rotate_num_error;

		posstion_num -= wc/erroe_num;
		rotate_num_error = wc%erroe_num;
		
		rotate_num_error2 += wc/erroe_num;/* 系数累加 */
		if(rotate_num_error2 >= 10) {
			rotate_num_error2 = 0;
			rotate_num_error2 -= 20;
		}

		WaitX(1000);
		moto_dr = 1;
		me.servo->speed_set(me.servo,-1000);
		while(posstion_num > 1) {
			WaitX(0);
		}
		me.servo->speed_set(me.servo,0);

//		WaitX(2);
//		if(me.button->get(me.button,B_SUB) == 0) {
//			moto_dr = 0;
//			me.servo->speed_set(me.servo,1000);
//		} else if(me.button->get(me.button,B_ADD) == 0) {
//			moto_dr = 1;
//			me.servo->speed_set(me.servo,-1000);
//		} else {
//			me.servo->speed_set(me.servo,0);
//		}
	
//		WaitX(5);
//		if(me.button->get(me.button,B_SUB) == 0) {
//			if(me.but_sub_count == 20) {
//				me.but_sub_count++;
//				servo_step(me.servo,me.power,S_ADD);
//			} else {
//				me.but_sub_count++;
//			}
//		} else {
//			me.but_sub_count = 0;
//		}

//		if(me.button->get(me.button,B_ADD) == 0) {
//			if(me.but_add_count == 20) {
//				me.but_add_count++;
//				servo_step(me.servo,me.power,S_SUB);
//			} else {
//				me.but_add_count++;
//			}
//		} else {
//			me.but_add_count = 0;
//		}
	}
fsm_end

int main() {
	//app_set();
	systick_config();
	device_init();

	fsm_task_on(LedTask);
	fsm_task_on(moto);
	while(1) {
		if(run_1ms_flag >= 1) {
			run_1ms_flag = 0;
			fsm_going(LedTask);
			fsm_going(moto);
		}
	}
	return 0;
}
