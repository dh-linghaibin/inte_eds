/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "flash.h"

#define PAGE_SIZE		((uint32_t)(1024))				   /* 一页的字节数 */
#define FLASH_START		((uint32_t)(0x08000000 + 0x04000)) /* flash获取地址120k */
static uint32_t page_merry[PAGE_SIZE/4];				   /* 内存缓冲 */

static uint32_t flash_read32(uint32_t address) {
  uint32_t temp1,temp2;
  temp1=*(__IO uint16_t*)address; 
  temp2=*(__IO uint16_t*)(address+2); 
  return (temp2<<16)+temp1;
}

static void flash_init(struct _flash_obj *flash) {
	//fmc_unlock();
}

static int flash_write(struct _flash_obj *flash,uint32_t address,uint32_t data) {
	uint16_t read_i = 0;
	uint32_t addr  = 0;
	uint8_t page_num = 0;
	uint16_t page_offset = 0;

	fmc_unlock();						  /* unlock the flash program/erase controller */
	page_num = (address/PAGE_SIZE);		  /* 计算第几页 */
	addr = (page_num*1024 + FLASH_START); /* 缓存块 */
	/* 读取页到缓存区 */
	do {
		page_merry[read_i] = flash_read32(addr);
		addr+=4;
	} while(++read_i < 256);
		
	fmc_page_erase(page_num*1024 + FLASH_START); /* 擦除 */
	page_offset = address%PAGE_SIZE;
	page_merry[page_offset] = data;
	addr = (page_num*1024 + FLASH_START);
	read_i = 0;
	do{
		fmc_word_program(addr,page_merry[read_i]);
		addr += 4;
	}while(++read_i < 256);
	
	fmc_lock(); /* lock the main FMC operation */
	return 0;
}

static int flash_read(struct _flash_obj *flash,uint32_t address,uint32_t *read_data) {
	uint32_t addr = (address/PAGE_SIZE)*1024 + FLASH_START + (address%PAGE_SIZE);
	*read_data = flash_read32(addr);
	return 0; 
}

void flash_rrgister(void) {
	struct _flash_obj *flash = GET_DAV(struct _flash_obj);

	flash->init = &flash_init;
	flash->write = &flash_write;
	flash->read = &flash_read;

    register_dev_obj("fla",flash);
}






