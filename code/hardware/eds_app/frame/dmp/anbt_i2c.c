/*************************************************************************************************************
圆点博士小四轴飞行器2014版配套源代码声明:
该源代码仅供参考,圆点博士不对源代码提供任何形式的担保,也不对因使用该源代码而出现的损失负责.
用户可以以学习的目的修改和使用该源代码.
但用户在修改该源代码时,不得移除该部分版权信息，必须保留原版声明.

更多信息，请访问官方网站www.etootle.com, 官方博客:http://weibo.com/xiaosizhou
**************************************************************************************************************/
#include "anbt_i2c.h"

void AnBT_DMP_Delay_us(uint32_t dly)
{
	uint8_t i;
	while(dly--) for(i=0;i<10;i++);
}
//
void AnBT_DMP_Delay_ms(uint32_t dly)
{
	while(dly--) AnBT_DMP_Delay_us(1000);
}
//

uint8_t AnBT_DMP_I2C_Write(uint8_t anbt_dev_addr, uint8_t anbt_reg_addr, uint8_t anbt_i2c_len, uint8_t *anbt_i2c_data_buf)
{		

		/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, anbt_dev_addr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, anbt_reg_addr);
	/* wait until the TBE bit is set */
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	
	for(int i = 0;i < anbt_i2c_len;i++) {
		/* data transmission */
		i2c_data_transmit(I2C0, anbt_i2c_data_buf[i]);
		/* wait until the TBE bit is set */
		while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	}
	 /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
	return 0x00;
}
uint8_t AnBT_DMP_I2C_Read(uint8_t anbt_dev_addr, uint8_t anbt_reg_addr, uint8_t anbt_i2c_len, uint8_t *anbt_i2c_data_buf) {
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, anbt_dev_addr, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
	/* data transmission */
	i2c_data_transmit(I2C0, anbt_reg_addr);
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
    i2c_master_addressing(I2C0, anbt_dev_addr, I2C_RECEIVER);
	  /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	
	/* While there is data to be read */
	while(anbt_i2c_len) {
		if(anbt_i2c_len == 1) {
			/* Disable Acknowledgement */
			i2c_ack_config(I2C0, I2C_ACK_DISABLE);	

			/* Send STOP Condition */
			i2c_stop_on_bus(I2C0);
		}
		/* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
		/* Read a byte from the MPU6050 */
		*anbt_i2c_data_buf = i2c_data_receive(I2C0);
		/* Point to the next location where the byte read will be saved */
		anbt_i2c_data_buf++;
		/* Decrement the read bytes counter */
		anbt_i2c_len--;
	}
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    return 0x00;
}
