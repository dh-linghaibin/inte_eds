/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _IIC_H_
#define _IIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32_config.h"

void iic_init(void);
void iic_write_byte(uint8_t slaveAddr,uint8_t pBuffer,uint8_t writeAddr);
void iic_read_byte(uint8_t slaveAddr, uint8_t* pBuffer, uint8_t readAddr, uint8_t NumByteToRead) ;


#ifdef __cplusplus
}
#endif

#endif
