

#include "mpu6050.h"


/** Get raw 6-axis motion sensor readings (accel/gyro).
 * Retrieves all currently available motion sensor values.
 * @param AccelGyro 16-bit signed integer array of length 6
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
static void _mpu6050_get_raw_accel_gyro(struct _mpu6050_obj *mpu)  {
    uint8_t tmpBuffer[14],i; 
    iic_read_byte(0xd0, tmpBuffer, MPU6050_RA_ACCEL_XOUT_H, 14); 
    /* Get acceleration */
    for(i=0; i<3; i++) 
      mpu->accgyo[i]=((int16_t)((uint16_t)tmpBuffer[2*i] << 8) + tmpBuffer[2*i+1]);
   /* Get Angular rate */
    for(i=4; i<7; i++)
      mpu->accgyo[i-1]=((int16_t)((uint16_t)tmpBuffer[2*i] << 8) + tmpBuffer[2*i+1]);        
}


static void _mpu6050_init(struct _mpu6050_obj *mpu) {
	iic_init();
	
	iic_write_byte(0xd0,0x00,MPU6050_RA_PWR_MGMT_1);
	iic_write_byte(0xd0,0x07,MPU6050_RA_SMPLRT_DIV);
	iic_write_byte(0xd0,0x06,MPU6050_RA_CONFIG);
	iic_write_byte(0xd0,0x01,MPU6050_RA_ACCEL_CONFIG);
	iic_write_byte(0xd0,0x18,MPU6050_RA_GYRO_CONFIG);
}

static void _mpu6050_power_off(struct _mpu6050_obj *mpu) {
	iic_write_byte(0xd0,0x40,MPU6050_RA_PWR_MGMT_1);
}


void mpu6050_register(void) {
	struct _mpu6050_obj *mpu6050 = GET_DAV(struct _mpu6050_obj);

	mpu6050->init      = &_mpu6050_init;
	mpu6050->get_rcg   = &_mpu6050_get_raw_accel_gyro;
	mpu6050->power_off = &_mpu6050_power_off;

    register_dev_obj("mpu",mpu6050);
}
