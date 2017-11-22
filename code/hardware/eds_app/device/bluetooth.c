/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */
#include "bluetooth.h"

#if PTR

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f) {
    usart_data_transmit(USART2, (uint8_t)ch);
    while (RESET == usart_flag_get(USART2, USART_FLAG_TBE));
    return ch;
}

#endif

#define ARRAYNUM(arr_nanme)      (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define TRANSMIT_SIZE            (ARRAYNUM(txbuffer) - 1)

uint8_t txbuffer[] = "\n\rUSART interrupt test\n\r";
uint8_t rxbuffer[32];
uint8_t tx_size = TRANSMIT_SIZE;
uint8_t rx_size = 32;
__IO uint8_t txcount = 0; 
__IO uint16_t rxcount = 0; 

static void bluetooth_init(struct _bluetooth_obj *blue) {
	/* USART interrupt configuration */
    nvic_irq_enable(USART2_IRQn, 0, 0);
    /* configure COM1 */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);
    /* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, BAUD);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
    /* enable USART TBE interrupt */  
    //usart_interrupt_enable(USART2, USART_INT_TBE);
    /* wait until USART send the transmitter_buffer */
    //printf("bluetooth_init successfully!\n\r");
	
	
	rcu_periph_clock_enable(RCU_AF);								  /* enable gpio af */
	gpio_pin_remap_config(GPIO_PD01_REMAP,ENABLE);					  /* set PD0 PD1 use gpio */
    rcu_periph_clock_enable(RCU_GPIOD);								  /* enable GPIO clock */
	gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0); /* init blue enable io */
	blue->power_off(blue);											  /* close blue */
}

int bluetooth_power_off(struct _bluetooth_obj *m) {
	/* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_11);

	GPIO_BOP(GPIOD) = GPIO_PIN_0; /* enable blue */
	return D_OK;
}

int bluetooth_power_on(struct _bluetooth_obj *m) {
	/* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

	GPIO_BC(GPIOD) = GPIO_PIN_0; /* enable blue */
	return D_OK;
}

void bluetooth_register(void) {
	struct _bluetooth_obj *ble = GET_DAV(struct _bluetooth_obj);

	ble->init      = &bluetooth_init;
	ble->power_off = &bluetooth_power_off;
	ble->power_on  = &bluetooth_power_on;

    register_dev_obj("ble",ble);
}

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART2_IRQHandler(void) {
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE)){
        /* receive data */
        rxbuffer[rxcount++] = usart_data_receive(USART2);
        if(rxcount == rx_size){
            usart_interrupt_disable(USART2, USART_INT_RBNE);
        }
    }
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_TBE)){
        /* transmit data */
        usart_data_transmit(USART2, txbuffer[txcount++]);
        if(txcount == tx_size){
            usart_interrupt_disable(USART2, USART_INT_TBE);
        }
    }
}
