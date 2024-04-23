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

#ifndef _FAT32_
#define _FAT32_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "sdc.h"
#include "uart0.h"

#include "iocompat.h"


#define MAX_NAME_SHORT_ENTRY 11
#define MAX_LONG_NAME 16 // the maximum bytes of the long name that we will be able to store

#define VOLUME_ATTR 0x08
#define LONG_NAME_ATTR 0x0F
#define DIRECTORY_ATTR 0x10
#define ARCHIVE_ATTR 0x20

#define FREE_LAST_ENTRY 0x00
#define FREE_USED_ENTRY 0xE5

#define MAX_VOLL_LAB 12

#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define MAX_MBR_PRIMARY_PARTITIONS 4
#define MBR_FIRST_PARTITION_ENTRY_OFFSET 0x1BE
#define MBR_PARTITION_ENTRY_LENGTH 16

typedef struct {
	uint8_t state;// 0- Not initialized 1-Initialized 2-Error'. Flag wich is set to 0,1,2... depending on the state of the FAT32 information
	uint32_t partition_first_sector;// sector where the FAT32 partition starts, it is something like the number of the zero sector of the FAT32 partion
	uint16_t bpb_bytspersec;// FAT32 BPB_BytsPerSec - 2 bytes
	uint8_t bpb_secperclus;// FAT32 BPB_SecPerClus - 1 byte
	uint16_t bpb_resvdseccnt;// FAT32 BPB_ResvdSecCnt - 2 bytes , it is the FAT start
	uint8_t bpb_numfats;// FAT32 BPB_NumFATs - 1 byte
	//NOT INDISPENSABLE uint32_t bpb_totsec32;// FAT32 BPB_TotSec32 - 4 bytes
	uint32_t bpb_fatsz32;// FAT32 BPB_FATSz - 4bytes
	uint32_t bpb_rootclus;// FAT32 BPB_RootClus - 4 bytes
	uint32_t bytes_per_cluster;// Precalculated number of bytes per cluster ( bpb_secperclus * bpb_secperclus ) - 4 bytes
	//NOT INDISPENSABLE uint8_t bs_vollab[MAX_VOLL_LAB];// FAT32 BS_VolLab - 11 bytes
	//NOT INDISPENSABLE  uint8_t bpb_filsystype[8];// FAT BPBFilSysTye - 8 bytes
	uint32_t first_data_region_sector;// number of the first sector of data region - 4 bytes
}t_fat32_info;


typedef struct {
	uint8_t type;// 0-It is  a root directory  1-It is  a directory 2-It is a file
	uint16_t number_of_files_dirs;// if we are in a directory it contains the number of files and directories
	uint8_t name[MAX_NAME_SHORT_ENTRY+1+1];// file or directory short name, +1+1 is for the '.' and the '\0'
	uint8_t previous_name[MAX_NAME_SHORT_ENTRY+1+1];// used to store the file or directory short name, +1+1 on which we were before going into the file or directory
	uint32_t first_fat_entry;// number of the first FAT entry of the directory or file. With this number and the information contained in FAT32_info structure can be read the content of the first FAT entry of the file or directory wich is being read
	uint32_t previous_first_fat_entry;// used to store the file or directory number of the first FAT entry of the directory or file on which we were before going into the file or directory. With this number and the information contained in FAT32_info structure can be read the content of the first FAT entry of the file or directory wich is being read
	uint32_t first_sector;// number of the first sector of the first FAT entry of the file or directory
	uint32_t size;//size in case it is a file
	uint32_t number_of_read_cluster;// number of cluster in the memory wich is being read
	uint32_t number_of_read_sector;// number of sector in the memory wich is being read
	uint16_t number_of_read_byte;// number of byte inside the sector wich is being read
	uint32_t last_read_byte_offset;// last read byte in the file, it is very usefull to know if is necessary to recalculate the adresses of cluster, sectors or bytes
}t_file_dir;

void FAT32_Reset();
uint32_t FAT32_ClusterFirstSector(uint32_t n_cluster);
int8_t FAT32_IsValidCluster(uint32_t n_cluster,uint32_t * entry_Value);
int8_t FAT32_Initialize();
int8_t FAT32_ReadFatEntry(uint32_t n_fat_entry_cluster, uint32_t * entry_value);
void FAT32_Test(int8_t step);
int8_t FAT32_ReadByte(uint8_t * read_byte);
int8_t FAT32_IsValidFirstSector();
int8_t FAT32_ReadByteOffset(uint32_t offset,uint8_t * read_byte);
int8_t FAT32_GetNumberDirElements(uint16_t * n_dirs_files);
int8_t FAT32_GetDirElementN(uint16_t entry_n,uint8_t * name,uint8_t * type, uint32_t * first_cluster,uint32_t * file_size,uint8_t * long_name);
void FAT32_GetShortDirEntry(uint32_t * address,uint8_t * name,uint8_t * type, uint32_t * first_cluster,uint32_t * file_size);
int8_t FAT32_LoadDirElement(uint32_t n_fat_entry_cluster,uint8_t type, uint8_t * name, uint32_t size);
int8_t FAT32_RestorePreviousDirElement();
void FAT32_GetLastReadByteOffset(uint32_t * byte_file_offset);
int8_t FAT32_SetReadByteOffset(uint32_t byte_file_offset);

#endif

