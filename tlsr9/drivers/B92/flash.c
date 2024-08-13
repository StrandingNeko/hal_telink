/********************************************************************************************************
 * @file    flash.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "lib/include/sys.h"
#include "lib/include/flash_base.h"
#include "lib/include/flash_prot.h"
#include "flash.h"

#include "mspi.h"
#include "timer.h"
#include "core.h"
#include "stimer.h"
#include "types.h"
#include "watchdog.h" //BLE SDK use
/*
 *	If add flash type, need pay attention to the read uid command and the bit number of status register
 *  Flash trim scheme has been added for P25Q80U.If other types of flash adds this scheme, user need to modify "flash_trim" and "flash_trim_check" function.
 *  When adding flash, if tRES1 is greater than 25us, update the delay of EFUSE_LOAD_AND_FLASH_WAKEUP_LOOP_NUM in the S file.
 *  If tRES1 is greater than 150us, this flash model cannot be used, because the chip hardware boot program only waits for 150us.
	Flash Type	uid CMD		MID		    Company		tRES1	Sector Erase Time(MAX)
	P25Q80U		0x4b		0x146085	PUYA		8us		20ms
	P25Q16SU    0x4b        0x156085    PUYA        8us		30ms
	P25Q32SU    0x4b        0x166085    PUYA        8us		30ms
	P25Q128L    0x4b        0x186085    PUYA        8us		30ms
    GD25LQ16E   0x4b        0x1560c8    GD          20us	300ms/500ms(85 centigrade/125 centigrade)
 */
unsigned int flash_support_mid[] = {0x146085,0x156085,0x166085,0x186085,0x1560c8,0x1660c8};
const unsigned int FLASH_CNT = sizeof(flash_support_mid)/sizeof(*flash_support_mid);



_attribute_data_retention_sec_ flash_handler_t flash_read_page = flash_dread;
_attribute_data_retention_sec_ flash_handler_t flash_write_page = flash_page_program;
/*
 * note:flash_write_page_encrypt and flash_read_page_decrypt_check should be used in combination,and the check read address is corresponding to the encrypted write.
 */
_attribute_data_retention_sec_ flash_handler_t flash_write_page_encrypt = flash_page_program_encrypt;
_attribute_data_retention_sec_ flash_read_check_handler_t flash_read_page_decrypt_check = flash_read_data_decrypt_check;



_attribute_data_retention_sec_ preempt_config_t s_flash_preempt_config =
{
	.preempt_en =0,
	.threshold  =1,
};

/*******************************************************************************************************************
 *												Primary interface
 ******************************************************************************************************************/
/**
 * @brief 		This function serves to set priority threshold. When the interrupt priority is greater than the maximum of the current interrupt threshold and the given interrupt threshold, flash process will disturb by interrupt.
 * @param[in]   preempt_en	- 1 can disturb by interrupt, 0 can disturb by interrupt.
 * @param[in]	threshold	- priority Threshold, .
 * @return    	none.
 * @note
 *              -# The correlation between flash_plic_preempt_config() and the flash functions that call sub-functions (all declared in flash_base.h) is as follows:
 *                  - When preempt_en = 1 and interrupt nesting is enabled (plic_preempt_feature_en):
 *                      - During the flash functions execution, the threshold of the PLIC is set to the maximum of the threshold before calling the interface and the given threshold value. \n
 *                        This means that when the external interrupt priority is greater than this maximum value, the execution of the flash function is disturbed by this interrupt.
 *                      - machine timer and software interrupt will definitely interrupt the flash functions execution, they are not controlled by the plic interrupt threshold
 *                  - In other cases(preempt_en = 0 or plic_preempt_feature_en = 0), global interrupts (including machine timer and software interrupt) will be turned off during the execution of the flash functions and will be restored when the flash functions exits.
 *              -# If the flash operation may be interrupted by an interrupt, it is necessary to ensure that the interrupt handling function and the function it calls must be in the RAM code. 
 */
void flash_plic_preempt_config(unsigned char preempt_en,unsigned char threshold)
{
	s_flash_preempt_config.preempt_en=preempt_en;
	s_flash_preempt_config.threshold=threshold;
}


/**
 * @brief 		This function serves to erase a sector.
 * @param[in]   addr	- must be 0 or a multiple of 0x1000.
 * @return 		none.
 * @note        Attention: The block erase takes a long time, please pay attention to feeding the dog in advance.
 * 				The maximum block erase time is listed at the beginning of this document and is available for viewing.
 *
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_erase_sector(unsigned long addr)
{
	wd_clear(); //BLE SDK use: clear watch dog
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_write_ram(FLASH_SECT_ERASE_CMD, addr,NULL,0);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}


/**
 * @brief 		This function reads the content from a page to the buf with single mode.
 * @param[in]   addr	- the start address of the page.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  buf		- the start address of the buffer(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:1x, dummy:0
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_read_data(unsigned long addr, unsigned long len, unsigned char *buf)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_READ_CMD,addr, buf,len);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}


/**
 * @brief 		This function reads the content from a page to the buf with dual read mode.
 * @param[in]   addr	- the start address of the page.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  buf		- the start address of the buffer(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:2x, dummy:8
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_dread(unsigned long addr, unsigned long len, unsigned char *buf)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_DREAD_CMD,addr, buf,len);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}

/**
 * @brief 		This function reads the content from a page to the buf with 4*IO read mode.
 * @param[in]   addr	- the start address of the page.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  buf		- the start address of the buffer(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:4x, data:4x, dummy:6
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_4read(unsigned long addr, unsigned long len, unsigned char *buf)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_X4READ_CMD, addr,  buf,len);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}


/**
 * @brief 		This function serves to decrypt the read data from the flash at the specified address and compare it with the plain text in single mode.
 * @param[in]   addr	        - the start address of the page.
 * @param[in]   plain_len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  plain_buf		- the start address of the plain buffer(ram address).
 * @return 		0: check pass; 1: check fail.
 * @note        cmd:1x, addr:1x, data:1x, dummy:0
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ unsigned char  flash_read_data_decrypt_check(unsigned long addr,unsigned long plain_len,unsigned char* plain_buf)
{
	unsigned char check_data=0;
     __asm__("csrci 	0x7d0,8");	//disable BTB
     check_data = flash_mspi_read_decrypt_check_ram(FLASH_READ_CMD,addr,plain_buf,plain_len);
     __asm__("csrsi 	0x7d0,8");	//enable BTB
     return check_data;
}
/**
 * @brief 		This function serves to decrypt the read data from the flash at the specified address and compare it with the plain text in dual read mode.
 * @param[in]   addr	        - the start address of the page.
 * @param[in]   plain_len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  plain_buf		- the start address of the plain buffer(ram address).
 * @return 		0: check pass; 1: check fail.
 * @note        cmd:1x, addr:1x, data:2x, dummy:8
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ unsigned char flash_dread_decrypt_check(unsigned long addr,unsigned long plain_len,unsigned char* plain_buf)
{
	unsigned char check_data=0;
	 __asm__("csrci 	0x7d0,8");	//disable BTB
	 check_data = flash_mspi_read_decrypt_check_ram(FLASH_DREAD_CMD,addr,plain_buf,plain_len);
	 __asm__("csrsi 	0x7d0,8");	//enable BTB
	 return check_data;
}
/**
 * @brief 		This function serves to decrypt the read data from the flash at the specified address and compare it with the plain text in 4*IO read mode.
 * @param[in]   addr	        - the start address of the page.
 * @param[in]   plain_len		- the length(in byte, must be above 0) of content needs to read out from the page.
 * @param[out]  plain_buf		- the start address of the plain buffer(ram address).
 * @return 		0: check pass; 1: check fail.
 * @note        cmd:1x, addr:4x, data:4x, dummy:6
 * 				Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ unsigned char flash_4read_decrypt_check(unsigned long addr,unsigned long plain_len,unsigned char* plain_buf)
{
	unsigned char check_data=0;
	 __asm__("csrci 	0x7d0,8");	//disable BTB
	 check_data = flash_mspi_read_decrypt_check_ram(FLASH_X4READ_CMD,addr,plain_buf,plain_len);
	 __asm__("csrsi 	0x7d0,8");	//enable BTB
	 return check_data;
}



/**
 * @brief 		This function writes the buffer's content to the flash.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @param[in]   cmd		- the write command. FLASH_WRITE_CMD or FLASH_QUAD_PAGE_PROGRAM_CMD.
 * @return 		none.
 */
_attribute_text_sec_ static void flash_write(unsigned long addr, unsigned long len, unsigned char *buf, flash_command_e cmd)
{
	unsigned int ns = PAGE_SIZE - (addr & (PAGE_SIZE-1));
	int nw = 0;

	while(len > 0){
		nw = len > ns ? ns : len;
		__asm__("csrci 	0x7d0,8");	//disable BTB
		flash_mspi_write_ram(cmd,addr,buf, nw);
		__asm__("csrsi 	0x7d0,8");	//enable BTB
		ns = PAGE_SIZE;
		addr += nw;
		buf += nw;
		len -= nw;
	}
}

/**
 * @brief 		This function writes the buffer's content to the flash with single mode.It is necessary to confirm whether this area
 * 				has been erased before writing. If not, you need to erase and then write, otherwise the write may be unsuccessful
 * 				and the data will become the wrong value. Note that when erasing, the minimum is erased by sector (4k bytes).
 * 				Do not erase the useful information in other locations of the sector during erasing.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:1x
 * 				the function support cross-page writing,which means the len of buf can bigger than 256.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_page_program(unsigned long addr, unsigned long len, unsigned char *buf)
{
	flash_write(addr, len, buf, FLASH_WRITE_CMD);
}

/**
 * @brief 		This function writes the buffer's content to the flash with quad page program mode.It is necessary to confirm whether this area
 * 				has been erased before writing. If not, you need to erase and then write, otherwise the write may be unsuccessful
 * 				and the data will become the wrong value. Note that when erasing, the minimum is erased by sector (4k bytes).
 * 				Do not erase the useful information in other locations of the sector during erasing.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:4x
 * 				the function support cross-page writing,which means the len of buf can bigger than 256.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_quad_page_program(unsigned long addr, unsigned long len, unsigned char *buf)
{
	flash_write(addr, len, buf, FLASH_QUAD_PAGE_PROGRAM_CMD);
}

/**
 * @brief 		This function writes the buffer's content to the flash in encrypt mode.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @param[in]   cmd		- the write command. FLASH_WRITE_CMD or FLASH_QUAD_PAGE_PROGRAM_CMD.
 * @return 		none.
 */
_attribute_text_sec_ static void flash_write_encrypt(unsigned long addr, unsigned long len, unsigned char *buf, flash_command_e cmd)
{
	unsigned int ns = PAGE_SIZE - (addr & (PAGE_SIZE-1));
	int nw = 0;

	while(len > 0){
		nw = len > ns ? ns : len;
		__asm__("csrci 	0x7d0,8");	//disable BTB
		flash_mspi_write_encrypt_ram(cmd,addr,buf, nw);
		__asm__("csrsi 	0x7d0,8");	//enable BTB
		ns = PAGE_SIZE;
		addr += nw;
		buf += nw;
		len -= nw;
	}
}

/**
 * @brief 		This function writes the buffer's content to the flash with single mode in encrypt mode.It is necessary to confirm whether this area
 * 				has been erased before writing. If not, you need to erase and then write, otherwise the write may be unsuccessful
 * 				and the data will become the wrong value. Note that when erasing, the minimum is erased by sector (4k bytes).
 * 				Do not erase the useful information in other locations of the sector during erasing.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:1x
 * 				the function support cross-page writing,which means the len of buf can bigger than 256.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_page_program_encrypt(unsigned long addr, unsigned long len, unsigned char *buf)
{
	flash_write_encrypt(addr, len, buf, FLASH_WRITE_CMD);
}

/**
 * @brief 		This function writes the buffer's content to the flash with quad page program mode in encrypt mode.It is necessary to confirm whether this area
 * 				has been erased before writing. If not, you need to erase and then write, otherwise the write may be unsuccessful
 * 				and the data will become the wrong value. Note that when erasing, the minimum is erased by sector (4k bytes).
 * 				Do not erase the useful information in other locations of the sector during erasing.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte, must be above 0) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into(ram address).
 * @return 		none.
 * @note        cmd:1x, addr:1x, data:4x
 * 				the function support cross-page writing,which means the len of buf can bigger than 256.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_quad_page_program_encrypt(unsigned long addr, unsigned long len, unsigned char *buf)
{
	flash_write_encrypt(addr, len, buf, FLASH_QUAD_PAGE_PROGRAM_CMD);
}

/**
 * @brief		This function reads the status of flash.
 * @param[in] 	cmd	- the cmd of read status. FLASH_READ_STATUS_CMD_LOWBYTE or FLASH_READ_STATUS_CMD_HIGHBYTE.
 * @return 		the value of status.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_  unsigned char flash_read_status(flash_command_e cmd)
{
	unsigned char status = 0;
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(cmd,0,&status,1);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
	return status;
}

/**
 * @brief 		This function write the status of flash.
 * @param[in]  	type	- the type of status.8 bit or 16 bit.
 * @param[in]  	data	- the value of status.
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_write_status(flash_status_typedef_e type , unsigned short data)
{

	unsigned char buf[2];

	buf[0] = data;
	buf[1] = data>>8;
	__asm__("csrci 	0x7d0,8");	//disable BTB
	if(type == FLASH_TYPE_8BIT_STATUS){
		flash_mspi_write_ram(FLASH_WRITE_STATUS_CMD_LOWBYTE,0,buf, 1);
	}else if(type == FLASH_TYPE_16BIT_STATUS_ONE_CMD){
		flash_mspi_write_ram(FLASH_WRITE_STATUS_CMD_LOWBYTE,0,buf,2 );
	}else if(type == FLASH_TYPE_16BIT_STATUS_TWO_CMD){
		flash_mspi_write_ram(FLASH_WRITE_STATUS_CMD_LOWBYTE,0, (unsigned char *)&buf[0],  1);
		flash_mspi_write_ram(FLASH_WRITE_STATUS_CMD_HIGHBYTE,0, (unsigned char *)&buf[1],1);
	}
	__asm__("csrsi 	0x7d0,8");	//enable BTB

}

/**
 * @brief 		This function serves to read data from the Security Registers of the flash.
 * @param[in]   addr	- the start address of the Security Registers.
 * @param[in]   len		- the length(in byte, must be above 0) of the content to be read.
 * @param[out]  buf		- the starting address of the content to be read(ram address).
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_read_otp(unsigned long addr, unsigned long len, unsigned char* buf)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_READ_SECURITY_REGISTERS_CMD,addr, buf, len);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}

/**
 * @brief 		This function serves to write data to the Security Registers of the flash you choose.
 * @param[in]   addr	- the start address of the Security Registers.
 * @param[in]   len		- the length(in byte, must be above 0) of content to be written.
 * @param[in]   buf		- the starting address of the content to be written(ram address).
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_write_otp(unsigned long addr, unsigned long len, unsigned char *buf)
{
	unsigned int ns = PAGE_SIZE_OTP - (addr & (PAGE_SIZE_OTP - 1));
	int nw = 0;

	while(len > 0){
		nw = len > ns ? ns : len;
		__asm__("csrci 	0x7d0,8");	//disable BTB
		flash_mspi_write_ram(FLASH_WRITE_SECURITY_REGISTERS_CMD,addr,buf,nw);
		__asm__("csrsi 	0x7d0,8");	//enable BTB
		ns = PAGE_SIZE_OTP;
		addr += nw;
		buf += nw;
		len -= nw;
	}
}

/**
 * @brief 		This function serves to erase the data of the Security Registers that you choose.
 * @param[in]   addr	- the address that you want to erase.
 * @return 		none.
 * @Attention	Even you choose the middle area of the Security Registers,it will erase the whole area.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_erase_otp(unsigned long addr)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_write_ram(FLASH_ERASE_SECURITY_REGISTERS_CMD,addr,  NULL,0 );
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}

/**
 * @brief	  	This function serves to read MID of flash(MAC id). Before reading UID of flash,
 * 				you must read MID of flash. and then you can look up the related table to select
 * 				the idcmd and read UID of flash.
 * @return    	MID of the flash(4 bytes).
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ unsigned int flash_read_mid(void)
{
	unsigned int flash_mid = 0;
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_GET_JEDEC_ID,0, (unsigned char *)&flash_mid, 3);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
	return flash_mid;
}


/**
 * @brief	  	This function serves to read UID of flash.Before reading UID of flash, you must read MID of flash.
 * 				and then you can look up the related table to select the idcmd and read UID of flash.
 * @param[in] 	idcmd	- different flash vendor have different read-uid command. E.g: GD/PUYA:0x4B; XTX: 0x5A.
 * @param[in] 	buf		- store UID of flash.
 * @return    	none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_read_uid(unsigned char idcmd,unsigned char *buf)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	if(idcmd==((FLASH_READ_UID_CMD_GD_PUYA_ZB_TH>>16)&0xff))
	{
		flash_mspi_read_ram(FLASH_READ_UID_CMD_GD_PUYA_ZB_TH ,0, buf,16);
	}
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}





/**
 * @brief 		This function is used to update the configuration parameters of xip(eXecute In Place),
 * 				this configuration will affect the speed of MCU fetching,
 * 				this parameter needs to be consistent with the corresponding parameters in the flash datasheet.
 * @param[in]	config	- xip configuration,reference structure flash_command_e
 * @return none
 */
_attribute_ram_code_com_sec_noinline_ void flash_set_xip_config_sram(flash_command_e config)
{
	unsigned int r=plic_enter_critical_sec(s_flash_preempt_config.preempt_en,s_flash_preempt_config.threshold);

	mspi_stop_xip();
	reg_mspi_xip_config = config;
	CLOCK_DLY_5_CYC;

	mspi_set_xip_en();
	plic_exit_critical_sec(s_flash_preempt_config.preempt_en,r);
}
_attribute_text_sec_ void flash_set_xip_config(flash_command_e config)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_set_xip_config_sram(config);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}


/**
 * @brief 		This function is used to write the configure of the flash,P25Q16SU/P25Q32SU/P25Q128L uses this function.
 * @param[in]   cmd			- the write command.
 * @param[out]  data		- the start address of the data buffer.
 * @return 		none.
 * @note		important:  "data" must not reside at flash, such as constant string.If that case, pls copy to memory first before write.
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ void flash_write_config(flash_command_e cmd,unsigned char data)
{
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_write_ram(cmd,0, &data, 1);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
}

/**
 * @brief 		This function is used to read the configure of the flash,P25Q16SU/P25Q32SU/P25Q128L uses this function.
 * @return 		the value of configure.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ unsigned char  flash_read_config(void)
{
	unsigned char config=0;
	__asm__("csrci 	0x7d0,8");	//disable BTB
	flash_mspi_read_ram(FLASH_READ_CONFIGURE_CMD,0, &config,1);
	__asm__("csrsi 	0x7d0,8");	//enable BTB
	return config;
}
/********************************************************************************************************
 *									secondary calling function,
 *	there is no need to add an circumvention solution to solve the problem of access flash conflicts.
 *******************************************************************************************************/


/**
 * @brief		This function serves to read flash mid and uid,and check the correctness of mid and uid.
 * @param[out]	flash_mid	- Flash Manufacturer ID.
 * @param[out]	flash_uid	- Flash Unique ID.
 * @return		0: flash no uid or not a known flash model 	 1:the flash model is known and the uid is read.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
_attribute_text_sec_ int flash_read_mid_uid_with_check( unsigned int *flash_mid ,unsigned char *flash_uid)
{
	unsigned char no_uid[16]={0x51,0x01,0x51,0x01,0x51,0x01,0x51,0x01,0x51,0x01,0x51,0x01,0x51,0x01,0x51,0x01};
	unsigned int i,f_cnt=0;
	*flash_mid = flash_read_mid();

	for(i=0; i<FLASH_CNT; i++){
		if(flash_support_mid[i] == *flash_mid){
			flash_read_uid(((FLASH_READ_UID_CMD_GD_PUYA_ZB_TH>>16)&0xff), (unsigned char *)flash_uid);
			break;
		}
	}
	if(i == FLASH_CNT){
		return 0;
	}

	for(i=0; i<16; i++){
		if(flash_uid[i] == no_uid[i]){
			f_cnt++;
		}
	}

	if(f_cnt == 16){	//no uid flash
		return 0;
	}else{
		return 1;
	}
}

/**
 * @brief		This function serves to get flash vendor.
 * @param[in]	none.
 * @return		0 - err, other - flash vendor.
 */
unsigned int flash_get_vendor(unsigned int flash_mid)
{
	switch(flash_mid&0x0000ffff)
	{
	case 0x0000325E:
		return FLASH_ETOX_ZB;
	case 0x000060C8:
		return FLASH_ETOX_GD;
	case 0x00004051:
		return FLASH_ETOX_GD;
	case 0x00006085:
		return FLASH_SONOS_PUYA;
	case 0x000060EB:
		return FLASH_SONOS_TH;
	case 0x000060CD:
		return FLASH_SST_TH;
	default:
		return 0;
	}
}


/*******************************************************************************************************************
 *									This function serves to 1m area flash protection
 ******************************************************************************************************************/
#define FLASH_1M_ADR_OFFSET 		0x100000
#define FLASH_2M_ADR_OFFSET 		0x200000
#define FLASH_3M_ADR_OFFSET 		0x300000
#define FLASH_3M5_ADR_OFFSET 		0x380000

#define FLASH_ADR_OFFSET_SELECT		FLASH_1M_ADR_OFFSET

#define FLASH_PROTECT_BLOCK_SIZE	FLASH_LOCK_FW_LOW_1M

void flash_protection_lock_init(void)
{
	unsigned int app_lockBlock = FLASH_PROTECT_BLOCK_SIZE; // init is 1M, in the ble lib, actual area will be less than 1m, so we protect 1m.
	unsigned int flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);
	flash_lock(flash_lockBlock_cmd);
}

void flash_protection_lock_operation(unsigned int offset)
{
	/* no need to lock again, detect fw addr will unlock flash. */
	if(offset < FLASH_ADR_OFFSET_SELECT){
		flash_unlock();
	}
}

void flash_protection_unlock_operation(unsigned int offset)
{
	/* suppose we will operate lock area, it will do ota, will unlock first and not lock again until it will reboot. */
	if(offset < FLASH_ADR_OFFSET_SELECT){
		flash_unlock();
	}
}
