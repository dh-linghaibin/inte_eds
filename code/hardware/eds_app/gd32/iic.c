/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "iic.h"


/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rcu_config(void) {
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
}

/*!
    \brief      cofigure the GPIO ports.
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void) {
    /* I2C0 and I2C1 GPIO ports */
    /* connect PB6 to I2C0_SCL */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
}

/*!
    \brief      cofigure the I2C0 and I2C1 interfaces..
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void i2c_config(void) {
    /* I2C clock configure */
    i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
    /* I2C address configure */
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x82);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);	
}

void iic_init(void) {
	rcu_config();
	gpio_config();
	i2c_config();
}

void delay_us(uint32_t n)
{
	uint8_t j;
	while(n--)
	for(j=0;j<10;j++);
}
void delay_ms(uint32_t n)
{
	while(n--)
	delay_us(1000);
}

void get_ms(unsigned long *time)
{

}

bool i2cWriteBuffer(uint8_t addr, uint8_t reg, uint8_t len, uint8_t * data) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, addr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, reg);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	
	for(int i = 0;i < len;i++) {
		/* data transmission */
		i2c_data_transmit(I2C0, data[i]);
		/* wait until the TBE bit is set */
		while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	}
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
    return 1;
}

bool i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, addr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, reg);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
	
	
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
	/* send slave address to I2C bus */
    i2c_master_addressing(I2C0, addr, I2C_RECEIVER);
	  /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	
	/* While there is data to be read */
	while(len) {
		if(len == 1) {
			/* Disable Acknowledgement */
			i2c_ack_config(I2C0, I2C_ACK_DISABLE);	

			/* Send STOP Condition */
			i2c_stop_on_bus(I2C0);
		}
		/* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
		/* Read a byte from the MPU6050 */
		*buf = i2c_data_receive(I2C0);
		/* Point to the next location where the byte read will be saved */
		buf++;
		/* Decrement the read bytes counter */
		len--;
	}
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    return 1;
}

int8_t i2cwrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t * data)
{
	if(i2cWriteBuffer(addr,reg,len,data))
	{
		return 0;
	}
	else
	{
		return 0;
	}
}

int8_t i2cread(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	if(i2cRead(addr,reg,len,buf))
	{
		return 0;
	}
	else
	{
		return 0;
	}
}

void iic_write_byte(uint8_t slaveAddr,uint8_t pBuffer,uint8_t writeAddr) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, slaveAddr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, writeAddr);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, pBuffer);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
}

void iic_write_bytes(uint8_t slaveAddr,uint8_t *pBuffer,uint8_t writeAddr,uint8_t NumByteToRead) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, slaveAddr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, writeAddr);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	
	for(int i = 0;i < NumByteToRead;i++) {
		/* data transmission */
		i2c_data_transmit(I2C0, pBuffer[i]);
		/* wait until the TBE bit is set */
		while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	}
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
}

void iic_read_byte(uint8_t slaveAddr, uint8_t* pBuffer, uint8_t readAddr, uint8_t NumByteToRead) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, slaveAddr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, readAddr);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
	
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
	/* send slave address to I2C bus */
    i2c_master_addressing(I2C0, slaveAddr, I2C_RECEIVER);
	  /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	
	/* While there is data to be read */
	while(NumByteToRead) {
		if(NumByteToRead == 1) {
			/* Disable Acknowledgement */
			i2c_ack_config(I2C0, I2C_ACK_DISABLE);	

			/* Send STOP Condition */
			i2c_stop_on_bus(I2C0);
		}
		/* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
		/* Read a byte from the MPU6050 */
		*pBuffer = i2c_data_receive(I2C0);
		/* Point to the next location where the byte read will be saved */
		pBuffer++;
		/* Decrement the read bytes counter */
		NumByteToRead--;
	}
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}


/*!
    \brief      this function handles I2C0 event interrupt request exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C0_EV_IRQHandler(void) {

}

/*!
    \brief      this function handles I2C0 error interrupt request exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C0_ER_IRQHandler(void) {

}


























