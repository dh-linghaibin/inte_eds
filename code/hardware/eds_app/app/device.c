/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "device.h"

void device_init(void) {
	bluetooth_register();
	led_register();
	servo_register();
	signal_register();
	power_register();
	mpu6050_dmp_register();
	rtc_register();
	button_register();
	iwdg_register();
	flash_rrgister();

	bluetooth_obj *ble = get_device("ble");
	if(ble != NULL) {
		ble->init(ble);
		ble->power_on(ble);
	}

	led_obj *led = get_device("led");	
	if(led != NULL) {
		led->init(led);
		//led->set(led,TOGGLE,R);
	}

	button_obj *button = get_device("but");	
	if(button != NULL) {
		button->init(button);
	}

	servo_obj *servo = get_device("ser");
	if(servo != NULL) {
		servo->init(servo);
		//servo->power_off(servo);
	}

	signal_obj *signal = get_device("sig");
	if(signal != NULL) {
		signal->init(signal);
		//signal->power_off(signal);
	}
	
	power_obj *power = get_device("pow");
	if(power != NULL) {
		power->init(power);
		power->power_on(power);
	}

	rtc_obj *rtc = get_device("rtc");
	if(rtc != NULL) {
		//rtc->init(rtc);
	}

	iwdg_obj *iwdg = get_device("iwdg");
	if(iwdg != NULL) {
		//iwdg->init(iwdg);
	}

	mpu6050dmp_obj *mpu6050 = get_device("mpu");
	if(mpu6050 != NULL) {
		mpu6050->init(mpu6050);
//		mpu6050->power_off(mpu6050);
	}

	flash_obj *flash = get_device("fla");
	if(flash != NULL) {
		flash->init(flash);
	}

	//pmu_to_standbymode(WFI_CMD);
	//rcu_periph_clock_enable(RCU_PMU);
	//	pmu_wakeup_pin_enable();
}

