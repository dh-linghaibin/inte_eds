/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "power.h"
#include "s_delay.h"

#define BOARD_ADC_CHANNEL           ADC_CHANNEL_4
#define ADC_GPIO_PORT               GPIOA
#define ADC_GPIO_PIN                GPIO_PIN_4

static uint16_t adc_value[2];
/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rcu_config(void) {
    rcu_periph_clock_enable(RCU_GPIOA);  		 /* enable GPIOA clock */
	rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_ADC0); 			 /* enable ADC clock */
    rcu_periph_clock_enable(RCU_DMA0);  		 /* enable DMA0 clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8); /* config ADC clock */
}

/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void) {
    gpio_init(ADC_GPIO_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, ADC_GPIO_PIN);  /* config the GPIO as analog mode */
	gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ,GPIO_PIN_5);
}
/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void dma_config(void) {
    dma_parameter_struct dma_data_parameter; /* ADC_DMA_channel configuration */
    dma_deinit(DMA0, DMA_CH0);               /* ADC DMA_channel configuration */
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(&adc_value);
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;  
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = 2;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, dma_data_parameter);

    dma_circulation_enable(DMA0, DMA_CH0);
  
    dma_channel_enable(DMA0, DMA_CH0); /* enable DMA channel */
}

/*!
    \brief      configure the ADC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void adc_config(void) {
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);  /* ADC trigger config */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);                         /* ADC data alignment config */
    adc_mode_config(ADC_MODE_FREE);                                               /* ADC mode config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 2);                      /* ADC channel length config */
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_4, ADC_SAMPLETIME_239POINT5); /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_9, ADC_SAMPLETIME_239POINT5); /* ADC regular channel config */
    adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, 3);                  /* ADC discontinuous mode */
    adc_enable(ADC0); 															  /* enable ADC interface */
    sdelay_ms(1); 																  /* wait adc enable must*/
	adc_calibration_enable(ADC0); 												  /* ADC calibration and reset calibration */
    adc_dma_mode_enable(ADC0); 													  /* ADC DMA function enable */
}

static void power_init(struct _power_obj * power) {
    rcu_config();  			 /* system clocks configuration */
    gpio_config(); 			 /* GPIO configuration */
    dma_config();  			 /* DMA configuration */
    adc_config();  			 /* ADC configuration */
	power->power_off(power); /* close val */
}

static int power_power_off(struct _power_obj *power) {
	GPIO_BC(GPIOA) = GPIO_PIN_5; /* close val */
	return D_OK;
}

static int power_power_on(struct _power_obj *power) {
	GPIO_BOP(GPIOA) = GPIO_PIN_5; /* enable val */
	return D_OK;
}

uint16_t power_get_battery(struct _power_obj * power) {
	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL); /* ADC software trigger enable */
	return (uint16_t)( (990*adc_value[1])/4096);
}

void power_register(void) {
	struct _power_obj *power = GET_DAV(struct _power_obj);
	
	power->init        = &power_init;
	power->get_battery = &power_get_battery;
	power->power_off   = &power_power_off;
	power->power_on	   = &power_power_on;

    register_dev_obj("pow",power);
}

