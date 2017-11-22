/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "flash.h"


static void flash_init() {
	fmc_unlock();
}

uint32_t wp_value = 0xFFFFFFFF, protected_pages = 0x0;


 int flash_write(uint32_t address,uint16_t *in_buf,uint32_t size) {
	fmc_unlock();		/* unlock the flash program/erase controller */
	ob_unlock();
	/* clear all pending flags */
	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	   /* get pages write protection status */
    wp_value = ob_write_protection_get();
	fmc_page_erase(address);
	while(size) {
		int flag = 0;
		flag = fmc_halfword_program(address,*in_buf); /* FMC program a half word at the corresponding address 0x08004000*/
		if(FMC_READY != flag) {
			return flag;
		}
		address++;
		size--;
		in_buf++;
	}
	return 0;
}

 int flash_read(uint32_t address,uint16_t *in_buf,uint32_t size) {
	while(size) {
		*in_buf = *(__IO uint16_t*)address;
		
		address++;
		size--;
		in_buf++;
	}
	return 0; 
}

void flash_rrgister(void) {
	
}

#define PAGE_SIZE                     ((uint32_t)(1024))
#define NAND_FLASH_BASE_ADDRESS       ((uint32_t)(0x08000000 + 0x04000))

/*!
    \brief      read data from multiple blocks of nand flash
    \param[in]  pBuf: pointer to user buffer
    \param[in]  read_addr: address to be read
    \param[in]  block_size: size of block
    \param[in]  block_num: number of block
    \param[out] none
    \retval     status
*/
uint32_t  flash_read_multi_blocks (uint8_t *pBuf, uint32_t read_addr, uint16_t block_size, uint32_t block_num)
{
    uint32_t i;
    uint8_t *pSource = (uint8_t *)(read_addr + NAND_FLASH_BASE_ADDRESS);

    /* Data transfer */
    while (block_num--) {
        for (i = 0; i < block_size; i++) {
            *pBuf++ = *pSource++;
        }
    }

    return 0;
}

/*!
    \brief      write data to multiple blocks of flash
    \param[in]  pBuf: pointer to user buffer
    \param[in]  write_addr: address to be write
    \param[in]  block_size: block size
    \param[in]  block_num: number of block
    \param[out] none
    \retval     status
*/
uint32_t flash_write_multi_blocks (uint8_t *pBuf,
                           uint32_t write_addr,
                           uint16_t block_size,
                           uint32_t block_num)
{
    uint32_t i, page;
    uint32_t start_page = (write_addr / PAGE_SIZE) * PAGE_SIZE + NAND_FLASH_BASE_ADDRESS;
    uint32_t *ptrs = (uint32_t *)pBuf;

    page = block_num;

    for(; page > 0; page--){
        fmc_page_erase(start_page);

        i = 0;

        do{
            fmc_word_program(start_page, *ptrs++);
            start_page += 4;
        }while(++i < 256);
    }

    return 0;
}

