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
#include "sys.h"
#include "device.h"


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
		//me.mpu6050->get_pry(me.mpu6050);
		//printf("mpu : %f2    %f2    %f2  speed:%d  cadence:%d \r\n",me.mpu6050->yaw,me.mpu6050->pitch,me.mpu6050->roll,me.signal->get_speed(me.signal),me.signal->get_cadence(me.signal));
//		WaitX(250);  
//		printf("bettery: %d V \r\n",me.power->get_moto_current(me.power));
	}
fsm_end





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
	while(1) {
		WaitX(5);
		if(me.button->get(me.button,B_SUB) == 0) {
			if(me.but_sub_count < 51) {
				if(me.but_sub_count == 10) {
					me.but_sub_count++;
					me.servo->to_stall(me.servo,me.power,S_ADD);
				} else {
					me.but_sub_count++;
				}
			}
		} else {
			if( (me.but_sub_count >= 5) && (me.but_sub_count <= 40) ){
				me.but_sub_count = 0;
			
			} else if(me.but_sub_count == 51) {
				me.but_sub_count = 0;
				me.servo->speed_set(me.servo,0);
			}
			me.but_sub_count = 0;
		}

		if(me.button->get(me.button,B_ADD) == 0) {
			if(me.but_add_count < 51) {
				if(me.but_add_count == 10) {
					me.but_add_count++;
					me.servo->to_stall(me.servo,me.power,S_SUB);
				} else {
					me.but_add_count++;
				}
			}
		} else {
			if( (me.but_add_count >= 5) && (me.but_add_count <= 40) ){
				me.but_add_count = 0;
			
			} else if(me.but_add_count == 51) {
		
			}
			me.but_add_count = 0;
		}
	}
fsm_end

extern uint8_t run_1ms_flag;
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
//	return 0;
}
