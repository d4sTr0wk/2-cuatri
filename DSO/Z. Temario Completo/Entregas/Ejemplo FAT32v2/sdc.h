// *****************************************************************
// ****                FOR EDUCATIONAL PURPOSES ONLY 		****
// ****                      www.tolaemon.com                   ****
// ****                       Source code                       ****
// ****                       04/03/2009                        ****
// ****                     Jordi Bartolome                     ****
// ****                                                         ****
// **** IMPORTANT:                                              ****
// **** Using parts of this code means accepting all conditions ****
// **** exposed on the web: www.tolaemon.com                    ****
// *****************************************************************

#ifndef _SDC_
#define _SDC_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "iocompat.h"

#define SDMMC_SECTOR_SIZE 512
#define SDMMC_NULL_SECTOR 1073741824;// is the last possible sector in a SD_MMC card, the maximum value in a 32 bit unsigned integer
#define SDMMC_CARD_INITIALIZATION_RETRIES 128
#define SDMMC_WHILE_READING_REINITIALIZE_RETRIES 2


typedef struct {
	uint8_t sector_buffer[SDMMC_SECTOR_SIZE];
	uint32_t actual_sector;
}t_SDMMC_buffer;

void SDMMC_Init();
void SDMMC_Test(int8_t step);
void SDMMC_SendStatus(uint8_t * byte1,uint8_t * byte2);
uint8_t SDMMC_WaitFEDataToken( );
void SDMMC_send_command(uint8_t byte_0,uint8_t byte_1,uint8_t byte_2,uint8_t byte_3,uint8_t byte_4,uint8_t byte_5 );
uint8_t SDMMC_wait_answer ( );
uint8_t SDMMC_send_byte(uint8_t valor);
uint8_t SDMMC_InitializeCard();
uint8_t SDMMC_ReadByte(uint32_t sector,uint16_t offset,uint8_t * read_byte);

#endif
