/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "sys.h"

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


uint8_t run_1ms_flag = 0;
void SysTick_Handler(void) {
	run_1ms_flag++;
}


