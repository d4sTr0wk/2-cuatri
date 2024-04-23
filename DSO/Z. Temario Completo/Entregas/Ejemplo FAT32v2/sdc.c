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

#include "stdio.h"
#include "ayfwre.h"
#include "sdc.h"
#include "timers.h"
#include "uart0.h"

#define CLEAR_CS PORTB=PORTB&0xFE // enables the SDCard
#define SET_CS PORTB=PORTB|0x01 // disables the SDCard
	
t_SDMMC_buffer SDMMC_buffer;	


/*	Description:
	Initializes the SDMMC module, preparing all the microcontroller hardware ( SPI ) to work with an SD MMC Card
	Recives:
	Returns:
 */
void SDMMC_Init(){

	DDRB=DDRB|0x07;// Sets MOSI, SCK and !SS as output, others are not modified
	DDRB=DDRB&0xF7;// MISO is set as input, others are not modified
	SPSR=SPSR&0xFE;// Clear the SPI2X bit
	SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR1);// enables the SPI, configures it as Master, and sets CLK speed to Fosc/128
	
};// SDMMC_Init



/*	Description:
	Sends the recived byte "value" to the SD MMC Card through the SPI
	Recives:
		value: with the byte we want to send to the card
	Returns:
		By value:
			Returns the value recived into the SPI data register after the transmission
 */
uint8_t SDMMC_send_byte(uint8_t value){
	
	SPDR=value;
	while (!bit_is_set(SPSR,7)); // se espera hasta que no haya finalizado la transmision
	SPSR=SPSR&0x7F;
		
	return SPDR; // se retorna el valor cargado en el registro de datos tras la transmision 
	
}/* SDMMC_send_byte */



/*	Description:
	Lower level procedure wich wakes-up the SD MMC Card
	Recives:
	Returns:
 */
void SDMMC_enable(){
	uint8_t i;
	
	SET_CS;/*CS disabled*/	
	TIMERS_BlockingPause(4);
	for (i=0;i<20;i++){
		SDMMC_send_byte(0xFF); /* to wake-up MMC SD card's SPI 80 Clocks are sent */
	};/*for*/
	CLEAR_CS;/*CS enabled*/
	
	//TIMERS_BlockingPause(10000);
	
}/*SDMMC_enable*/



/*	Description:
	Low level procedure wich disables the SD MMC Card
	Recives:
	Returns:
 */
void SDMMC_disable(){

	SET_CS;/*CS desactivado*/	
//	SPCR=0x13;/* M A L !!!! REVISAR configuracion de la SPI: 0001 0011 */
		
}/*SDMMC_disable*/



/*	Description:
	Sends the command bytes byte_0,byte_1,byte_2 ... to the SD MMC Card through the SPI
	Recives:
		byte_0, byte_1, byte_2, byte_3, byte_4, byte_5: bytes of the command we want to send
	Returns:
 */
void SDMMC_send_command(uint8_t byte_0,uint8_t byte_1,uint8_t byte_2,uint8_t byte_3,uint8_t byte_4,uint8_t byte_5 ){
	
	SET_CS;/*CS disabled*/	
	TIMERS_BlockingPause(2);/* TIMERS_BlockingPause */		
	SDMMC_send_byte(0xFF);	
	CLEAR_CS;/*CS enabled*/		
	TIMERS_BlockingPause(2);/* TIMERS_BlockingPause */		
	/* Command is sent */	
	SDMMC_send_byte(byte_0);
	SDMMC_send_byte(byte_1);
	SDMMC_send_byte(byte_2);
	SDMMC_send_byte(byte_3);
	SDMMC_send_byte(byte_4);
	SDMMC_send_byte(byte_5);	
	TIMERS_BlockingPause(2);/* TIMERS_BlockingPause */		
	
}/* SDMMC_send_command */



/* funcion que retorna la respuesta de la MMCard al comando enviado */
uint8_t SDMMC_wait_answer ( ){
	uint8_t i=0;	
	uint8_t valor=0xFF;

	/* entre el comando y la respuesta existe un retardo que va de 1 a 8  bytes (yo pongo 32 (4x) para que 
	haya tiempo segurisisisimo), la respuesta R1 tiene una longitud de un byte y siempre comienza con un 0 */	
	while ( (i<=32) && (valor==0xFF) ){
		valor=SDMMC_send_byte(0xFF);	
		i++;	
	};/*while*/	
	
	return valor;

}/* SDMMC_wait_answer */



/* funcion que retorna la respuesta de la MMCard al comando enviado */
uint8_t SDMMC_WaitFEDataToken( ){
	uint8_t i=0;	
	uint8_t valor=0x00;

	/* se mantiene esperando el Data Token enviado por la tarjeta */
	while ( (i<=64) && (valor!=0xFE) ){
		valor=SDMMC_send_byte(0xFF);	
		i++;	
	};/*while*/	
	
	return valor;

}/* SDMMC_WaitFEDataToken */



/*	Description:
	Executes the SDMMC card intialization sequence: 1-GO_IDLE_STATE and 2-SEND_OP_COND to initialize the card
	Recives:
	Returns:
		By value:
			0 if has ocurred any error during the process and the card has NOT been initialized
			1 if there is no error and the card is initialized
 */
uint8_t SDMMC_InitializeCard(){
	uint8_t card_answer1;
	uint8_t card_answer2;
	uint8_t retrys;
	uint8_t return_value;

	SDMMC_disable();	
	retrys=SDMMC_CARD_INITIALIZATION_RETRIES;
	card_answer1=0;
	while ( (retrys>0)&&(card_answer1!=1) ){		
		SDMMC_enable();
		// CMD0 GO_IDLE_STATE Resets the SD Card
		SDMMC_send_command(0x40,0x00,0x00,0x00,0x00,0x95);
		card_answer1=SDMMC_wait_answer();
		SDMMC_disable();			
		retrys--;
	};// while

	// The host must poll the card (by repeatedly sending CMD1) until the ‘in-idle-state’ bit in the card response
	// indicates that the card completed it's initialization process and is ready for the next command
	card_answer2=1;
	while ( (retrys>0)&&(card_answer2!=0) ){
		SDMMC_enable();	
		// CMD1 SEND_OP_COND Activates the card’s initialization process.
		SDMMC_send_command(0x41,0x00,0x00,0x00,0x00,0xFF);
		card_answer2=SDMMC_wait_answer();
		SDMMC_disable();
		retrys--;
	};// while
	SDMMC_disable();						
	
	// si las respuestas recibidas son las correctas se retorna 1 sino 0
	if ((card_answer1==1)&&(card_answer2==0)){
		return_value=1;
		// inicializamos el buffer situando actual_sector apuntando al sector invalido
		SDMMC_buffer.actual_sector=SDMMC_NULL_SECTOR;		
	}else{
		return_value=0;
	};// if

	return return_value;
	
}// SDMMC_InitializeCard



/*	Description:
	Sends the SET_BLOCKLEN command which sets the length of the read or written blocks. 
	Recives:
	Returns:
		By value:
			the R1 response returned by the card ( 0 if there is no error, and !=0 if there is any error )
 */
uint8_t SDMMC_SetBlockLength(uint16_t length){
	uint8_t low;
	uint8_t high;
	uint8_t answer;
	
	low=(uint8_t)length&0x00FF;
	high=(uint8_t)(length&0xFF00)>>8;

	// CMD16 SET_BLOCKLEN Selects a block length (in bytes) for all following block commands (read & write)
	SDMMC_send_command(0x50,0x00,0x00,high,low,0xFF);// CMD16
	answer=SDMMC_wait_answer();

	return answer;

};// SDMMC_SetBlockLength



/*	Description:
	Sends the SEND_STATUS command to ask the status of the SD card
	Recives:
	Returns:
		By value:
		By reference:
			byte 1: with the first byte sent by the card
			byte 2: with the second byte sent by the card
 */
void SDMMC_SendStatus(uint8_t * byte1,uint8_t * byte2){
	
	// SEND_STATUS Asks the selected card to send its status register.
	SDMMC_send_command(0x42,0x00,0x00,0x00,0x00,0xFF);// CMD13
	*byte1=SDMMC_wait_answer();
	*byte2=SDMMC_wait_answer();
	
};// SDMMC_SendStatus



/*	Description:
	Writes each byte of the buffer into specified sector of the MMC SD Card
	Recives:
		sector: the number of sector where we want to store the buffer
	Returns:
		By value:
			0 if has ocurred any error during the process and has not been possible to store the buffer
			1 if the buffer has been stored into the sector without problems
		By reference:
 */
uint8_t SDMMC_WriteBufferIntoSector(uint32_t sector){
	uint32_t address=0;
	uint8_t address_byte_1=0x00;
	uint8_t address_byte_2=0x00;
	uint8_t address_byte_3=0x00;
	uint8_t card_answer;
	uint8_t return_value;
	uint8_t i_aux1,i_aux2;
	uint16_t i;
		
	address=(sector*SDMMC_SECTOR_SIZE);
	address_byte_1=(uint8_t)((address&(0x0000FF00))>>8);
	address_byte_2=(uint8_t)((address&(0x00FF0000))>>16);
	address_byte_3=(uint8_t)((address&(0xFF000000))>>24);
	
	//CMD16 SET_BLOCKLEN Block Length Command is sent before reading
	SDMMC_SetBlockLength(SDMMC_SECTOR_SIZE);
	
	//CMD24 WRITE_BLOCK Writes a block of the size selected by the SET_BLOCKLEN command.3
	SDMMC_send_command(0x58,address_byte_3,address_byte_2,address_byte_1,0x00,0xFF);// CMD24
	card_answer=SDMMC_wait_answer();
	
	// si R1 recibido es 0 entonces es que ha aceptado la operacion de escritura
	if (card_answer==0){
		// se envia el byte de inicio de trama
		card_answer=SDMMC_send_byte(0xFE);
		i=0;
		while (i<SDMMC_SECTOR_SIZE){
			i_aux1=SDMMC_buffer.sector_buffer[i];
			card_answer=SDMMC_send_byte(i_aux1);
			i++;
		};//while
		// two last CRC bytes are sent
		card_answer=SDMMC_send_byte(0xFF);
		card_answer=SDMMC_send_byte(0xFF);
		// SendStatus must be executed every time a block is writen
		SDMMC_SendStatus(&i_aux1,&i_aux2);
		return_value=1;
	}else{
		return_value=0;
	};//if
	
	return return_value;
	
};//SDMMC_WriteBufferIntoSector



/*	Description:
		Loads the content of specified SD-MMC card sector into sector_buffer
	Recives:
		sector: with the number of sector we want to load into the sector_buffer
	Returns:
		by value:
			0 if has ocurred any error during the process and has not been possible to load the buffer
			1 if the sector has been loaded into the buffer without problems
 */
uint8_t SDMMC_LoadSectorIntoBuffer(uint32_t sector){
	uint32_t address;
	uint8_t address_byte_1;
	uint8_t address_byte_2;
	uint8_t address_byte_3;
	uint8_t card_answer;
	uint8_t return_value;
	uint16_t i;
		
	address=(sector*SDMMC_SECTOR_SIZE);
	address_byte_1=(uint8_t)((address&(0x0000FF00))>>8);
	address_byte_2=(uint8_t)((address&(0x00FF0000))>>16);
	address_byte_3=(uint8_t)((address&(0xFF000000))>>24);

	//CMD16 SET_BLOCKLEN Block Length Command is sent before reading
	SDMMC_SetBlockLength(SDMMC_SECTOR_SIZE);

	//CMD17 READ_SINGLE_BLOCK Reads a block of the selected size
	SDMMC_send_command(0x51,address_byte_3,address_byte_2,address_byte_1,0x00,0xFF);// CMD17
	card_answer=SDMMC_wait_answer();

	// if R1 value is 0 it means that READ_SINGLE_BLOCK operation has been accepted by the car
	if (card_answer==0){	
		
		// waits the data block start token
		SDMMC_WaitFEDataToken();
		// start reciving data bytes
		i=0;
		while (i<SDMMC_SECTOR_SIZE){
			card_answer=SDMMC_send_byte(0xFF);
			SDMMC_buffer.sector_buffer[i]=card_answer;
			i++;		
		};//while
		// two last CRC bytes are recived
		card_answer=SDMMC_send_byte(0xFF);
		card_answer=SDMMC_send_byte(0xFF);
		SDMMC_buffer.actual_sector=sector;
		return_value=1;
		
	}else{
			
		return_value=0;
		
	};//if

	return return_value;

};//SDMMC_LoadSectorIntoBuffer



/*	Description:
		Returns by reference the byte stored into the specified sector and offset of the MMC SD 
		card. To optimize response time, first this routine checks if requested sector is already 
		stored into the sector_buffer. So, if requested sector content is sotred into the sector_buffer
		the offset byte ir read and returned by reference (it only takes an acces to memory). If the 
		requested sector is not stored into the sector_buffer it has to be loaded from the card 
		to memory, and then offset byte is read.
	Recives:
		sector: with the number of sector where the byte we want to read is located
		offset: with the offset into the sector where the byte we want to read is placed
	Returns:
		by value:
			0 if has ocurred any error during the reading process and has not been possible to read the byte
			1 if the byte has been read without problems
		by reference:
			read_byte: with the read value
 */
uint8_t SDMMC_ReadByte(uint32_t sector,uint16_t offset,uint8_t * read_byte){
	uint8_t return_value;
	uint8_t retrys;
	
	return_value=1;
	
	//checks if requested sector is already the actual sector stored into the buffer
	if (sector!=SDMMC_buffer.actual_sector){
		
		// if it is not actual sector we try to load it
		retrys=0;
		while ( (SDMMC_LoadSectorIntoBuffer(sector)!=1)&&(retrys<SDMMC_WHILE_READING_REINITIALIZE_RETRIES) ){
			// if the buffer cant be loaded, perhaps is due to the fact that SDMMC is not initialized yet
			SDMMC_InitializeCard();
			retrys++;
		};//while	

		// if has not been possible to load the buffer or initilize the card, retrun value is set to 0
		if (retrys>=SDMMC_WHILE_READING_REINITIALIZE_RETRIES){
			return_value=0;
		};//if
		
	};//if
	
	// once the buffer contains requested sector, offset byte is read
	if (return_value==1){
		
		// first we checks that the offset parameter is right
		if ((offset>=0)&&(offset<SDMMC_SECTOR_SIZE)){
			*read_byte=SDMMC_buffer.sector_buffer[offset];
		}else{
			return_value=0;
		};//if
		
	};//if

	return return_value;

};//SDMMC_ReadByte



/*	Description:
	Tests routines, which lets check if the SDMMC works properly or not
	Recives:
		step: with the test routine to execute
	Returns:
	  By value:
	  By reference:
 */
void SDMMC_Test(int8_t step){

	uint8_t i_aux;
	uint32_t  i;
	uint16_t  j;
	
	switch (step){

		case 0:
			// SDMMC_StoreBufferIntoSector test routine
			UART0_ForceTransmitByte('I');
			UART0_ForceTransmitByte('n');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('t');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('a');
			UART0_ForceTransmitByte('l');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('z');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('n');
			UART0_ForceTransmitByte('g');
			UART0_ForceTransmitByte(' ');
			if (SDMMC_InitializeCard()==1){
				UART0_PushByteIntoTransmissionBuffer('O');
				UART0_PushByteIntoTransmissionBuffer('K');
			}else{
				UART0_PushByteIntoTransmissionBuffer('K');
				UART0_PushByteIntoTransmissionBuffer('O');
			};//if
			break;
		case 1:
			// SDMMC_WriteBufferIntoSector test routine
			UART0_ForceTransmitByte('W');
			UART0_ForceTransmitByte('r');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('t');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('n');
			UART0_ForceTransmitByte('g');
			UART0_ForceTransmitByte(' ');
			for (i=0;i<10;i++){
				UART0_ForceTransmitByte('-');			
				// initializes the first 64 bytes of the buffer with 'i' char
				for (j=0;j<512;j++){
					SDMMC_buffer.sector_buffer[j]=(j%255);
					//UART0_ForceTransmitByte(48+j);
				};//for
				// stores the buffer into sector i
				if (SDMMC_WriteBufferIntoSector(i)==1){
					UART0_ForceTransmitByte(' ');
					UART0_ForceTransmitByte('O');
					UART0_ForceTransmitByte('K');
				}else{
					UART0_ForceTransmitByte(' ');
					UART0_ForceTransmitByte('K');
					UART0_ForceTransmitByte('O');				
				};//if
			};//next
			break;	
		case 2:
			// SDMMC_LoadSectorIntoBuffer test routine
			UART0_ForceTransmitByte('R');
			UART0_ForceTransmitByte('e');
			UART0_ForceTransmitByte('a');
			UART0_ForceTransmitByte('d');
			UART0_ForceTransmitByte('i');
			UART0_ForceTransmitByte('n');
			UART0_ForceTransmitByte('g');
			UART0_ForceTransmitByte(' ');
			for (i=0;i<1;i++){
				UART0_ForceTransmitByte('*');
				if (SDMMC_LoadSectorIntoBuffer(i)==1){
					UART0_ForceTransmitByte(' ');
					UART0_ForceTransmitByte('O');
					UART0_ForceTransmitByte('K');
					// shows the content of read buffer
					for (j=0;j<SDMMC_SECTOR_SIZE;j++){
						UART0_ForceTransmitByte(48+SDMMC_buffer.sector_buffer[j]);
					};//for
				}else{
					UART0_ForceTransmitByte(' ');
					UART0_ForceTransmitByte('K');
					UART0_ForceTransmitByte('O');				
				};//if
			};//next
			break;	
		case 3:
			// SDMMC_ReadByte test routine
			for (i=0;i<10;i++){
				UART0_ForceTransmitByte('#');
				for (j=0;j<SDMMC_SECTOR_SIZE;j++){
					if (SDMMC_ReadByte(i,j,&i_aux)==1){
						UART0_ForceTransmitByte(48+i_aux);
					}else{
						UART0_ForceTransmitByte('?');
					};//if
				};//next
			};//next			
			break;
		case 4:
			// First sectors DUMP routine ( to check FAT32 bytes structure ). 
			// A binary serial reciver is needed to read all bytes
			for (i=0;i<64;i++){
				// Before sending the block through hyperterminal, we send sector number
				UART0_ForceTransmitByte('#');
				UART0_ForceTransmitByte((i&0xFF000000)>>24);
				UART0_ForceTransmitByte((i&0x00FF0000)>>16);
				UART0_ForceTransmitByte((i&0x0000FF00)>>8);
				UART0_ForceTransmitByte((i&0x000000FF));
				UART0_ForceTransmitByte('#');
				// Sends the i sector content through hyperterminal
				for (j=0;j<SDMMC_SECTOR_SIZE;j++){
					if (SDMMC_ReadByte(i,j,&i_aux)==1){
						UART0_ForceTransmitByte(i_aux);
					}else{
						UART0_ForceTransmitByte('?');
					};//if
				};//next
			};//next			
			break;
		case 5:
			// First sectors DUMP routine ( to check FAT32 bytes structure ). 
			// A binary serial reciver is needed to read all bytes
			//for (i=3900;i<3940;i++){
			for (i=2000;i<2200;i++){
				// Before sending the block through hyperterminal, we send it's sector number
				UART0_ForceTransmitByte('#');
				UART0_ForceTransmitByte((i&0xFF000000)>>24);
				UART0_ForceTransmitByte((i&0x00FF0000)>>16);
				UART0_ForceTransmitByte((i&0x0000FF00)>>8);
				UART0_ForceTransmitByte((i&0x000000FF));
				UART0_ForceTransmitByte('#');
				// Sends the i sector content through hyperterminal
				for (j=0;j<SDMMC_SECTOR_SIZE;j++){
					if (SDMMC_ReadByte(i,j,&i_aux)==1){
						UART0_ForceTransmitByte(i_aux);
					}else{
						UART0_ForceTransmitByte('?');
					};//if
				};//next
			};//next			
			break;	
		default:
			break;
		};//switch

};//SDMMC__test
