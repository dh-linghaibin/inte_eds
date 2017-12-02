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
#include "flash.h"


/* 指示 */
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

/* 手动变速 */
simple_fsm(moto,
	uint8_t    but_sub_count;
	uint8_t    but_add_count;
	servo_obj  *servo;
	button_obj *button; 
	power_obj *power;
	flash_obj *flash;		)

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
	me.flash = get_device("fla");
	if(me.flash == NULL) {
		fsm_task_off(LedTask); /* get flash faild out the task */
		return 0;
	}
	uint32_t c_stall = 0;
	me.flash->read(me.flash,C_STALL,&c_stall);
	if(c_stall <= 10) {
		me.servo->stall = c_stall;
	} else {
		me.servo->stall = 0;
		me.flash->write(me.flash,C_STALL,me.servo->stall);
	}
	me.flash->read(me.flash,C_POSSTION_NUM,&c_stall);
	if(c_stall <= 6000) {
		me.servo->set_posstion(me.servo,c_stall);
	} else {
		me.servo->set_posstion(me.servo,0);
		me.flash->write(me.flash,C_POSSTION_NUM,me.servo->get_posstion(me.servo));
	}
	printf("stall %d \r\n",me.servo->stall);
	printf("get_posstion %d \r\n",me.servo->get_posstion(me.servo));
	while(1) {
		WaitX(5);
		if(me.button->get(me.button,B_SUB) == 0) {
			if(me.but_sub_count < 51) {
				if(me.but_sub_count == 10) {
					me.but_sub_count++;
					switch(me.servo->to_stall(me.servo,me.power,S_ADD)) {
						case 0:/* 换挡成功 */
							me.flash->write(me.flash,C_STALL,me.servo->stall);
						break;
						case 1:/* 电流过流 */
							printf("error 1 \r\n");
						break;
						case 2:/* 时间超时 */
							printf("error 2 \r\n");
						break;
						case 3:/* 在限位 */
							printf("error 3 \r\n");
						break;
					}
					WaitX(250);
					me.flash->write(me.flash,C_POSSTION_NUM,me.servo->get_posstion(me.servo));
					printf("posstion_num %d \r\n",me.servo->get_posstion(me.servo));
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
					switch( me.servo->to_stall(me.servo,me.power,S_SUB) ) {
						case 0:/* 换挡成功 */
							me.flash->write(me.flash,C_STALL,me.servo->stall);
						break;
						case 1:/* 电流过流 */
							printf("error 1 \r\n");
						break;
						case 2:/* 时间超时 */
							printf("error 2 \r\n");
						break;
						case 3:/* 在限位 */
							printf("error 3 \r\n");
						break;
					}
					WaitX(250);
					me.flash->write(me.flash,C_POSSTION_NUM,me.servo->get_posstion(me.servo));
					printf("posstion_num %d \r\n",me.servo->get_posstion(me.servo));
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

/* 自动变速测试 */
simple_fsm(t_auto,
	mpu6050dmp_obj *mpu6050;
	signal_obj *signal;
	servo_obj  *servo;
	power_obj *power;
	flash_obj *flash;
	button_obj *button; 
	uint8_t but_sub_count;
)

fsm_init_name(t_auto)
	me.mpu6050 = get_device("mpu");
	if(me.mpu6050 == NULL) {
		fsm_task_off(t_auto); /* get mpu6050 faild out the task */
		return 0;
	}
	me.signal = get_device("sig");
	if(me.signal == NULL) {
		fsm_task_off(t_auto); /* get signal faild out the task */
		return 0;
	}
	me.servo = get_device("ser");
	if(me.servo == NULL) {
		fsm_task_off(t_auto); /* get servo faild out the task */
		return 0;
	}
	me.power = get_device("pow");
	if(me.power == NULL) {
		fsm_task_off(t_auto); /* get power faild out the task */
		return 0;
	}
	me.flash = get_device("fla");
	if(me.flash == NULL) {
		fsm_task_off(t_auto); /* get flash faild out the task */
		return 0;
	}
	me.button = get_device("but");	
	if(me.button == NULL) {
		fsm_task_off(moto); /* get button faild out the task */
		return 0;
	}
	while(1) {
		WaitX(250);  
//		printf("signal %d \r\n",me.signal->get_speed(me.signal));
//		printf("cadence %d \r\n",me.signal->get_cadence(me.signal));
		me.mpu6050->get_pry(me.mpu6050);
//		if(me.mpu6050->pitch > 0) {
//			printf("上坡 %f2\r\n",me.mpu6050->pitch);
//		} else {
//			printf("下坡 %f2\r\n",me.mpu6050->pitch);
//		}
		if(me.button->get(me.button,B_SUB) == 0) {
			if(me.but_sub_count < 3) {
				if(me.but_sub_count == 2) {
					me.but_sub_count++;
					me.mpu6050->zero(me.mpu6050);
					printf("归零 \r\n");
				} else {
					me.but_sub_count++;
				}
			}
		} else {
			me.but_sub_count = 0;
		}
		printf("mpu : %f2    %f2    %f2  speed:%d  cadence:%d \r\n",me.mpu6050->yaw,me.mpu6050->pitch,me.mpu6050->roll,me.signal->get_speed(me.signal),me.signal->get_cadence(me.signal));
	}
fsm_end

extern uint8_t run_1ms_flag;
int main() {
	//app_set();
	systick_config();
	device_init();

	fsm_task_on(LedTask);
	fsm_task_on(t_auto);
	//fsm_task_on(moto);
	while(1) {
		if(run_1ms_flag >= 1) {
			run_1ms_flag = 0;
			fsm_going(LedTask);
			fsm_going(moto);
			fsm_going(t_auto);
		}
	}
//	return 0;
}
