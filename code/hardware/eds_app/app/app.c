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
#include "rtc.h"
#include "l_math.h"
#include "iwdg.h"
#include "s_delay.h"
#include "l_os.h"

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


unsigned int spPA5[128];
unsigned int spPA6[128];
unsigned int spPA7[128];
struct task taskPA5,taskPA6,taskPA7;

void PA5On(void)
{
	while(1)
	{
		led_obj *led = get_device("led");	
		if(led != NULL) {
			led->set(led,TOGGLE,R);
		}
		sleep(200);
		if(led != NULL) {
			led->set(led,TOGGLE,R);
		}
		sleep(200);
	}
}

void PA6On(void)
{
	while(1)
	{
		led_obj *led = get_device("led");	
		if(led != NULL) {
			led->set(led,TOGGLE,SIGNAL);
		}
		sleep(150);
		if(led != NULL) {
			led->set(led,TOGGLE,SIGNAL);
		}
		sleep(150);
	}
}

void PA7On(void)
{
	while(1)
	{
		led_obj *led = get_device("led");	
		if(led != NULL) {
			led->set(led,TOGGLE,B);
		}
		sleep(250);
		if(led != NULL) {
			led->set(led,TOGGLE,B);
		}
		sleep(250);
	}
}

static void device_init(void) {
	bluetooth_register();
	led_register();
	servo_register();
	signal_register();
	power_register();
	mpu6050_register();
	rtc_register();
	iwdg_register();

	bluetooth_obj *ble = get_device("ble");
	if(ble != NULL) {
		ble->init(ble);
		ble->power_off(ble);
	}

	led_obj *led = get_device("led");	
	if(led != NULL) {
		led->init(led);
		//led->set(led,TOGGLE,R);
	}

	servo_obj *servo = get_device("ser");
	if(servo != NULL) {
		servo->init(servo);
		servo->power_off(servo);
	}

	signal_obj *signal = get_device("sig");
	if(signal != NULL) {
		signal->init(signal);
		signal->power_off(signal);
	}
	
	power_obj *power = get_device("pow");
	if(power != NULL) {
		power->init(power);
		power->power_off(power);
	}

	rtc_obj *rtc = get_device("rtc");
	if(rtc != NULL) {
		//rtc->init(rtc);
	}

	iwdg_obj *iwdg = get_device("iwdg");
	if(iwdg != NULL) {
		//iwdg->init(iwdg);
	}

	mpu6050_obj *mpu6050 = get_device("mpu");
	if(mpu6050 != NULL) {
//		mpu6050->init(mpu6050);
//		mpu6050->power_off(mpu6050);
	}
}

int main() {
	//app_set();
	systick_config();
	/* clock enable */
//	rcu_periph_clock_enable(RCU_PMU);
//	pmu_wakeup_pin_enable();

	device_init();

	//while(1);
	//AnBT_DMP_MPU6050_Init();//6050DMP初始化

	//fsm_task_on(LedTask);
	//fsm_task_on(mpu);
	//sdelay_ms(1000);
	//pmu_to_deepsleepmode(PMU_LDO_LOWPOWER,WFI_CMD);
	//led->set(led,TOGGLE,R);
	//pmu_to_standbymode(WFI_CMD);
	//pmu_to_deepsleepmode(PMU_LDO_NORMAL,WFI_CMD);
	SystemInit();
	//led_register();
	led_obj *led2 = get_device("led");	
	if(led2 != NULL) {
		led2->init(led2);
		led2->set(led2,TOGGLE,R);
	}
	
//	if(mpu6050 != NULL) {
//		//mpu6050->power_off(mpu6050);
//	}
	iwdg_obj *iwdg = get_device("iwdg");
//	while(1) {
//		if(led2 != NULL) {
//			led2->set(led2,TOGGLE,R);
//		}
//		sdelay_ms(500);
//		
//		if(iwdg != NULL) {
//			iwdg->reload(iwdg);
//		}
//	}
//	pmu_to_standbymode(WFI_CMD);

//	sdelay_ms(300);
//	printf("in seleep \r\n");
//	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
//	ble->init(ble);
//	printf("out seleep \r\n");
//	led->set(led,TOGGLE,R);
	
	
	taskCreate(&taskPA5,31,PA5On,spPA5,128,"PA5");
	taskCreate(&taskPA6,30,PA6On,spPA6,128,"PA6");
	taskCreate(&taskPA7,29,PA7On,spPA7,128,"PA7");

	QSysStart();
	return 0;
}
