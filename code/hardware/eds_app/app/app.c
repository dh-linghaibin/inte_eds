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
#include "mpu6050.h"
#include "signal.h"
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
	led_obj   *led;	
	power_obj *power; )
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
	while(1) {
		WaitX(250);  
		me.led->set(me.led,TOGGLE,R);
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
	posstion_num = 300;
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


static const uint16_t stall[11] = {2000,2500,3000,3500,4000,4500,5000,5500,6000,6600,7000};

void to_pos(servo_obj  *servo,uint8_t stal) {
	if(stall[stal] > posstion_num) {
		moto_dr = 0;
		servo->speed_set(servo,1000);
		while(posstion_num < 600) {
			uint16_t last = 600 - posstion_num;
			if(last > 100) {
				servo->speed_set(servo,800);
			} else if(last > 80) {
				servo->speed_set(servo,700);
			} else if(last > 60) {
				servo->speed_set(servo,500);
			} else if(last > 40) {
				servo->speed_set(servo,400);
			} else if(last > 20) {
				servo->speed_set(servo,300);
			}
		}
		servo->speed_set(servo,0);
	} else {
		
	}
}

uint16_t last;

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
		moto_dr = 0;
		me.servo->speed_set(me.servo,1000);
		uint8_t breaks = 0;
		while(posstion_num < (600-breaks)) {
			WaitX(0);
			last = me.power->get_moto_current(me.power);
			if(last > 280) {
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
		
		WaitX(1000);
		moto_dr = 1;
		me.servo->speed_set(me.servo,-1000);
		while(posstion_num > 35) {
			WaitX(0);
		}
		me.servo->speed_set(me.servo,0);
		
//		WaitX(2);
//		if(me.button->get(me.button,SUB) == 0) {
//			moto_dr = 0;
//			me.servo->speed_set(me.servo,1000);
//		} else if(me.button->get(me.button,ADD) == 0) {
//			moto_dr = 1;
//			me.servo->speed_set(me.servo,-1000);
//		} else {
//			me.servo->speed_set(me.servo,0);
//		}

//		if(me.button->get(me.button,SUB) == 0) {
//			if(me.but_sub_count == 30) {
//				me.but_sub_count++;
//				moto_dr = 0;
//				me.servo->speed_set(me.servo,1000);
//				while(posstion_num < 590) {
//					WaitX(0);
//					me.power->get_moto_current(me.power);
//					uint16_t last = 600 - posstion_num;
//					if(last > 100) {
//						me.servo->speed_set(me.servo,800);
//					} else if(last > 80) {
//						me.servo->speed_set(me.servo,700);
//					} else if(last > 60) {
//						me.servo->speed_set(me.servo,500);
//					} else if(last > 40) {
//						me.servo->speed_set(me.servo,400);
//					} else if(last > 20) {
//						me.servo->speed_set(me.servo,300);
//					}
//				}
//				me.servo->speed_set(me.servo,0);
//			} else {
//				me.but_sub_count++;
//			}
//		} else {
//			me.but_sub_count = 0;
//		}

//		if(me.button->get(me.button,ADD) == 0) {
//			if(me.but_add_count == 30) {
//				me.but_add_count++;
//				moto_dr = 1;
//				me.servo->speed_set(me.servo,-1000);
//				while(posstion_num > 20) {
//					WaitX(0);
//				}
//				me.servo->speed_set(me.servo,0);
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
