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

#include "fat32.h"

t_fat32_info FAT32_info; // general information of the FAT32 structure
t_file_dir FAT32_file_dir; // general information about the read file or directory



/*	Description:
	Resets all FAT32 internal structures
	Recives:
	Returns:
*/
void FAT32_Reset(){

	FAT32_info.state=0;// 0- Not initialized 1-Initialized 2-Error. Flag wich is set to 0,1,2... depending on the state of the FAT32 information
	FAT32_info.partition_first_sector=0;// sector where the FAT32 partition starts
	FAT32_info.bpb_bytspersec=0;// FAT32 BPB_BytsPerSec - 2 bytes
	FAT32_info.bpb_secperclus=0;// FAT32 BPB_SecPerClus - 1 byte
	FAT32_info.bpb_resvdseccnt=0;// FAT32 BPB_ResvdSecCnt - 2 bytes
	FAT32_info.bpb_numfats=0;// FAT32 BPB_NumFATs - 1 byte
	//NOT INDISPENSABLE uint32_t bpb_totsec32;// FAT32 BPB_TotSec32 - 4 bytes
	FAT32_info.bpb_fatsz32=0;// FAT32 BPB_FATSz - 4bytes
	FAT32_info.bpb_rootclus=0;// FAT32 BPB_RootClus - 4 bytes
	//NOT INDISPENSABLE uint8_t bs_vollab[MAX_VOLL_LAB];// FAT32 BPB_VolLab - 11 bytes
	//NOT INDISPENSABLE uint8_t bpb_filsystype[8];// FAT BPBFilSysTye - 8 bytes
	FAT32_info.bytes_per_cluster=0;// Precalculated number of bytes per cluster ( bpb_secperclus * bpb_secperclus ) - 4 bytes

	FAT32_file_dir.type=0;
	FAT32_file_dir.number_of_files_dirs=0;
	FAT32_file_dir.name[0]='\0';// file or directory short name
	FAT32_file_dir.previous_name[0]='\0';// file or directory short name
	FAT32_file_dir.first_fat_entry=0;// number of the first FAT entry of the directory or file. With this number and the information contained in FAT32_info structure can be read the content of the first FAT entry of the file or directory wich is being read
	FAT32_file_dir.previous_first_fat_entry=0;// number of the first FAT entry of the directory or file. With this number and the information contained in FAT32_info structure can be read the content of the first FAT entry of the file or directory wich is being read
	FAT32_file_dir.first_sector=0;// number of the first sector of the first FAT entry of the file or directory
	//FAT32_file_dir.first_cluster=0;// number of the first cluster of the file or directory
	FAT32_file_dir.number_of_read_cluster=0;// number of cluster in the memory wich is being read
	FAT32_file_dir.number_of_read_sector=0;// number of sector in the memory wich is being read
	FAT32_file_dir.number_of_read_byte=0;// number of byte inside the sector wich is being read
	FAT32_file_dir.last_read_byte_offset=0;// last read byte in the file, it is very usefull to know if is necessary to recalculate the adresses of cluster, sectors or bytes;// contains wich of the cluster of the file is being read, 0,1,2,3,4,5...N it is used to avoid recalculating the cluster if we havent moved from the cluster
	FAT32_file_dir.size=0;
	
};//FAT32_Reset



/*	Description:
	Returns the file offset of last read byte
	Recives:
	Returns:
		By reference:
			uint32_t with the position inside the file, of the last read byte
*/

void FAT32_GetLastReadByteOffset(uint32_t * byte_file_offset){

	*byte_file_offset=FAT32_file_dir.last_read_byte_offset;
	
};//FAT32_GetLastReadByteOffset



/*	Description:
	Sets the file reading pointer into the position especfied by byte_file_offset var. This
	function code is nearly the same than the ReadByteOffset
	Recives:
	
	Returns:
		By reference:
		By value:
			1: if the read pointer has been placed in the specified position
			-1: if has not been possible to place the pointer into the specified position
*/
int8_t FAT32_SetReadByteOffset(uint32_t byte_file_offset){
	int8_t ret;
	uint32_t aux_double_word;
	uint32_t aux_double_word2;
	
	ret=1;
	
	// first we check that FAT32 has been initialized, and the cluster is a valid cluster
	if (FAT32_info.state==1){
		
		// gets the number of clusters behind the offset byte
		aux_double_word=byte_file_offset /  (FAT32_info.bytes_per_cluster);
		aux_double_word2=FAT32_file_dir.last_read_byte_offset / (FAT32_info.bytes_per_cluster);
		
		// OPTIMIZATION:if the number of clusters behind the offset bytes is diferent to the number of 
		// clustersbehind the byte read last time, it means that the cluster to read is diferent, and we
		// have to calculate it's address again. But if it is the same, it means we calculated it previously
		// and it is no necessary to recalculate its cluster address, because it is already stored 
		// into "FAT32_file_dir.number_of_read_cluster" structure
		if (aux_double_word!=aux_double_word2){
	
			// OPTIMIZATION: if the number of clusters behind the "offset" byte is larger than the number of 
			// clusters behind the byte read last time we don't have to go back to the begining of the file counting 
			// from the first cluster, we can continue calculating from last read cluster, it avoids recalculating 
			// all previously recalculated clusters each time we read in a new cluster that is behind the last 
			// read one. In the other side, if the number of clusters behind the "offset" byte is lower than the number
			// of previously calculated cluster we recalculate again its cluster from the begining
			if (aux_double_word>aux_double_word2){
				// the number of cluster jumps decresases from (aux_double_word) to (aux_double_word-aux_double_word2)
				// what is faster than excuete aux_double_word again
				aux_double_word=aux_double_word-aux_double_word2;
			}else{
				// the number of cluster jumps is aux_double_word
				FAT32_file_dir.number_of_read_cluster=FAT32_file_dir.first_fat_entry;
			};//if
			
			// looks for the "offset byte" cluster 
			while((aux_double_word!=0)&&(ret>0)){
					
				// reads the cluster entry and checks it, if it is ok aux_double_word2 will contain the entry
				if (FAT32_IsValidCluster(FAT32_file_dir.number_of_read_cluster,&aux_double_word2)>=0){				
					FAT32_file_dir.number_of_read_cluster=aux_double_word2;// if entry is valid we jump to it
					aux_double_word--;// the var wich controls the numer of remaining jumps is decreased
				}else{
					// we are in an invalid cluster
					FAT32_file_dir.number_of_read_cluster=FAT32_file_dir.first_fat_entry;// reset read cluster to the first cluster of the file or directory
					ret=-3;
				};//if
				
			};//while
	
		};//if
		
		if (ret==1){
			// gets the sector of the "offset byte" 
			FAT32_file_dir.number_of_read_sector=FAT32_ClusterFirstSector(FAT32_file_dir.number_of_read_cluster);
			
			aux_double_word=byte_file_offset % FAT32_info.bytes_per_cluster;
			FAT32_file_dir.number_of_read_sector=FAT32_file_dir.number_of_read_sector + (aux_double_word / FAT32_info.bpb_bytspersec);
			
			// gets the number of byte inside the sector wich corresponds to the "offset byte"
			FAT32_file_dir.number_of_read_byte=byte_file_offset%FAT32_info.bpb_bytspersec;
			
			FAT32_file_dir.last_read_byte_offset=byte_file_offset;
			
		};//if
	
	}else{
		// FAT32_fil_dir has not been initi
		ret=-1;
	};//if
	
	return ret;

};//FAT32_SetReadByteOffset




/*	Description:
	Reads the FAT entry related to cluster N. In other words it reads FAT entry N
	Recives:
		By value:
			uint32_t with the number of fat entry (cluster) we want to read
	Returns:
		By value:
			 1: if the FAT entry has been read without problems
			-1: any error has ocurred while reading the FAT entry
		By reference:
			uint32_t with the FAT read value
*/
int8_t FAT32_ReadFatEntry(uint32_t n_fat_entry_cluster, uint32_t * entry_value){
	int8_t ret;
	uint8_t aux_byte;
	uint16_t aux_offset;
	uint32_t aux_sector;
	uint32_t aux_double_word;


	// first we check that FAT32 has been initialized, and the cluster is a valid cluster
	if ( (FAT32_info.state==1)
			&&(n_fat_entry_cluster>=FAT32_info.bpb_rootclus)
			&&(n_fat_entry_cluster<=0x0FFFFFEF))
		{	
		// First we calculate sector number and offset of the entry
		//   FATSz = BPB_FATSz32; 
		//   FATOffset = N * 4; 
		//   ThisFATSecNum = BPB_ResvdSecCnt + (FATOffset / BPB_BytsPerSec); 
		//   ThisFATEntOffset = REM(FATOffset / BPB_BytsPerSec); 
		aux_sector = FAT32_info.partition_first_sector + FAT32_info.bpb_resvdseccnt + (n_fat_entry_cluster*4 / FAT32_info.bpb_bytspersec );
		aux_offset = ( n_fat_entry_cluster*4 ) % FAT32_info.bpb_bytspersec ;
		
		// Loads Fat Entry
		*entry_value=0;
		ret=SDMMC_ReadByte(aux_sector,aux_offset,&aux_byte);
		*entry_value=aux_byte;
		ret=ret&SDMMC_ReadByte(aux_sector,aux_offset+1,&aux_byte);
		aux_double_word=aux_byte;
		*entry_value=*entry_value|(aux_double_word<<8);
		ret=ret&SDMMC_ReadByte(aux_sector,aux_offset+2,&aux_byte);
		aux_double_word=aux_byte;
		*entry_value=*entry_value|(aux_double_word<<16);
		ret=ret&SDMMC_ReadByte(aux_sector,aux_offset+3,&aux_byte);
		if (ret==1){
			aux_double_word=aux_byte;
			*entry_value=*entry_value|(aux_double_word<<24);
		}else{
			ret=-1;
		};//if
	}else{
		ret=-1;
	};//if
	
	return ret;
	
};//FAT32_ReadFatEntry



/*	Description:
	Recives a number of cluster, checks it's entry in the FAT and returns if
	it is a valid or invalid cluster
	Recives:
		Is necessary that FAT32_info structure has been previously intialized, 
		otherway used values will be incorrect
		By value:
			uint32_t with the number of cluster
	Returns:
		By value:
		 2: is a valid but last cluster
		 1: is a valid cluster
		-1: error at entry
		-2: is an empty cluster
		-3: is a reserved cluster
*/
int8_t FAT32_IsValidCluster(uint32_t n_cluster,uint32_t * entry_Value){
	int8_t ret;
	
	ret=FAT32_ReadFatEntry(n_cluster,entry_Value);
	if (ret<0){
		// error when reading entry
		ret=-1;
	}else{
		// only the 28 lower bits are taken
		*entry_Value=*entry_Value&0x0FFFFFFF;
		// check the value of the read entry to validate or not the cluster
		if ((*entry_Value>=FAT32_info.bpb_rootclus)&&(*entry_Value<=0x0FFFFFEF)){
			// n_cluster corresponds to a valid cluster			
			ret=1;
		}else if(*entry_Value==0x0FFFFFFF){
			// n_cluster corresponds to a valid but last cluster of the file or directory
			ret=2;
		}else if(*entry_Value==0x0){
			// n_cluster is an empty cluster (0x00000000)
			ret=-2;
		}else{
			// n_cluster corresponds to a reserved cluster
			ret=-3;
		};//if
	};//if
	return ret;

};//FAT32_IsValidCluster



/*	Description:
	Recives a number of cluster and returns the number of sector where it starts
	Recives:
		Is necessary that FAT32_info structure has been previously intialized, 
		otherway used values will be incorrect
		By value:
			uint32_t with the number of cluster
	Returns:
		By value:
			uint32_t with the number of sector
*/
uint32_t FAT32_ClusterFirstSector(uint32_t n_cluster){
	uint32_t aux_double_word;

	aux_double_word=FAT32_info.first_data_region_sector + ((n_cluster-FAT32_info.bpb_rootclus) * FAT32_info.bpb_secperclus);

	return aux_double_word;

};//FAT32_ClusterFirstSector



/*	Description:
	Returns the number of files and dirs contained in a directory
	Recives:
		The FAT32_file_dir structure must contain directory data, not file data
		otherway it wont be able to return the number of entries in the directory
		Obviously it is also necessary that FAT32_info structure has been previously
		intialized, otherway used values will be incorrect
	Returns:
		By reference:
			uint16_t * with the number of file or dir entries in the directory
		By value:
			int8_t with the result of the operation
					 1: the number of entries has been read
					-1: FAT32_file_dir is not initialized
					-2: FAT32_file_dir invalid information ( FAT32_file_dir perhaps corresponds to a file and so it does not have entries )
					-3: an error has ocured while reading the memory
*/
int8_t FAT32_GetNumberDirElements(uint16_t * n_dirs_files){
	int8_t ret;
	uint32_t aux_doubleword;
	uint8_t read_byte;
	uint8_t exit;

	// first we check that FAT32 has been initialized
	if  (FAT32_info.state!=1){
		ret=-1;
	}else{
	
		// cheks if FAT32_file_dir strucutre is loaded with file information
		if (FAT32_file_dir.type==2){
			ret=-2;
		}else{
		
			// jumps to the first byte in the directory
			if (FAT32_file_dir.type==0){
				// is a root directory
				aux_doubleword=32;// jumps the first two 32 bytes of the volume entry
			}else{
				// is a standar directory
				aux_doubleword=0;
			};//if
			
			// starts processing the directory entries
			(*n_dirs_files)=0;
			exit=0;
			do{
				// reads the first byte of the entry
				ret=FAT32_ReadByteOffset(aux_doubleword,&read_byte);
				if (ret<0){
					// an error has ocured while reading the memory
					ret=-3;
					exit=1;
				}else{
					// process the first byte of each entry to get more information
					switch (read_byte){
						case FREE_LAST_ENTRY:
							// al entries have been read without problems
							ret=1;
							exit=1;	
							break;
						case FREE_USED_ENTRY:
							aux_doubleword=aux_doubleword+32;//jumps the entry
							break;
						default:
							// reads the DIR_Attr byte of the entry
							ret=FAT32_ReadByteOffset(aux_doubleword+11,&read_byte);				
							// checks in wich kind of entry is the read pointer placed
							switch (read_byte){
								case VOLUME_ATTR:
									aux_doubleword=aux_doubleword+32;//jumps the entry
									break;
								case LONG_NAME_ATTR:
									// it should be a first long entry of a file or directory
									if ((read_byte&0x40)==0x40){
										// it corresponds to a first "Long Directory entry" (LDIR_Ord=0x40)
										read_byte=read_byte&0x3F;// removes 3 highest bits to get the number of "Long directory entries"
										// jumps all the Long Directory Entry Structures. 
										// LONG ENTRY CODE SHOULD BE PLACED HERE FILLING THE LONG NAME STRINT INSTEAD OF JUMPING THE ENTRIES
										aux_doubleword=aux_doubleword+(32*read_byte);
									}else{
										// we should never enter here because all long entries of a file or directory are treated toghether when
										// read the firs entyr, but I place this code just in case
										aux_doubleword=aux_doubleword+32;//jumps the longentry
									};//if
									break;
								case DIRECTORY_ATTR:
								case ARCHIVE_ATTR:
									// next bytes should correspond to a "(short) Directory entry"
									aux_doubleword=aux_doubleword+32;
									// increases in one the number of File or Directory entries
									(*n_dirs_files)++;	
									break;
							};//switch
							break;
					};//switch
					
				};//if
			}while(exit==0);
		};//if
	};//if
	
	return ret;

};//FAT32_GetNumberDirElements



/*	Description:
	Processes the content of a set of Long Directory Entries of a long name. Reads all
	de characters contained in the entries and stores them into the long name string. Despite
	they are unicode chars, they are treated as simple chars taking only the first byte.
	Recives:
		Obviously FAT32_file_dir structure must be initialized and contain
		directory data, not file data, otherway it wont find entries to read
		Also, the read pointer recived in "address" must be placed at the first
		byte of the first entry of the set of Long Directory Entries.
		By value:
			address: with the address (offset byte in the clsuter) where
			the first of the set of Long Entries starts.
	Returns:
		By reference:
			long_name: a MAX_LONG_NAME string with the long name of the file or directory 
		By value:
			int8_t with the result of the operation
					 1: the entry has been read
					-1: an error ocurred while reading the entries
*/
int8_t FAT32_GetLongDirEntry(uint32_t * address,uint8_t * long_name){
	int8_t ret;
	uint8_t read_byte;
	uint8_t numb_long_entries;
	uint8_t entries_counter;
	uint8_t i,j;
	uint32_t base_address;
	
	long_name[0]='\0';
	
	ret=FAT32_ReadByteOffset(*address,&read_byte);
		
	// checks if it is a first "Long Directory Entry"  of a file or directory
	if ((read_byte&0x40)!=0x40){
		// it is not a first "Long Directory Entry" we should never enter here because
		// all long entries of a file or directory are treated toghether when read the first
		// entry, but I place this code just in case
		*address=(*address)+32;//jumps the longentry
	}else{
		// it corresponds to a first "Long Directory entry" (LDIR_Ord=0x40)
		entries_counter=read_byte&0x3F;// removes 3 highest bits to get the number of "Long directory entries"
		
		numb_long_entries=entries_counter;// stores the number of entries to use them after reading all the entries
		base_address=*address;// stores the address first address
		
		// reads all the long name entries starting by the last one because it contains the begining of the long name
		i=0;// is used as index to fill the string
		while ((ret>=0)&&(entries_counter>0)){
			*address=base_address+((entries_counter-1)*32);// places the read pointer at the begining of the LONG NAME ENTRY
			(*address)++;// jumps the LDIR_Ord byte										
			// reads and stores this entry LDIR_Name1 in the long_name string
			j=0;
			while ((ret>=0)&&(entries_counter>0)&&(j<5)&&(i<MAX_LONG_NAME)){
				ret=FAT32_ReadByteOffset(*address,&read_byte);
				if (read_byte==0x00){
					// it means we have arrived to the last charcater and force the exit
					entries_counter=0x00;
				}else{
					long_name[i]=read_byte;
					i++;
					(*address)++;
					// we don't use the second UNICODE byte
					(*address)++;
				};//while
				j++;
			};//while
				
			// jumps the LDIR_Attr , LDIR_Type and LDIR_Chksum bytes
			*address=(*address)+3;
			
			// reads and stores this entry LDIR_Name2 in the long_name string behind the LDIR_Name1
			j=5;
			while ((ret>=0)&&(entries_counter>0)&&(j<11)&&(i<MAX_LONG_NAME)){
				ret=FAT32_ReadByteOffset(*address,&read_byte);
				if (read_byte==0x00){
					// it means we have arrived to the last charcater and force the exit
					entries_counter=0x00;
				}else{
					long_name[i]=read_byte;	
					i++;
					(*address)++;
					// we don't use the second UNICODE byte
					(*address)++;
				};//while
				j++;
			};//while

			// jumps the LDIR_FstClusLO bytes
			*address=(*address)+2;
												
			// reads and stores this entry LDIR_Name3 in the long_name string behind the LDIR_Name1
			j=11;
			while ((ret>=0)&&(entries_counter>0)&&(j<13)&&(i<MAX_LONG_NAME)){
				ret=FAT32_ReadByteOffset(*address,&read_byte);
				if (read_byte==0x00){
					// it means we have arrived to the last charcater and force the exit
					entries_counter=0x00;
				}else{
					long_name[i]=read_byte;	
					i++;
					(*address)++;
					// we don't use the second UNICODE byte
					(*address)++;
				};//while
				j++;
			};//while
			if (entries_counter!=0) entries_counter--;// decreases the var wich allows controll the iterations number
				
		};//while
		long_name[i]='\0';

		
		// if an error has occurred the return var is set to the propper valu
		if (ret<0){
			ret=-1;
		};//if
	
		// places the read pointer behind the last LONG DIR ENTRY to continue reading
		*address=base_address+(numb_long_entries*32);

	};//if
	
	return ret;
};//FAT32_GetLongDirEntry



/*	Description:
	Processes the content of a (short) Directory Entry: it stores all de data
	contained on the entry, stores it into the vars and returns them by reference.
	Recives:
		Obviously FAT32_file_dir structure must be initialized and contain
		directory data, not file data, otherway it wont find entries to read
		Also, the read pointer recived in "address" must be placed at the first
		byte of the entry.
		By value:
			address: with the address (offset byte in the clsuter) where
			the entry starts.
	Returns:
		By reference:
			name: a string with the name of the file or directory 
			type: with the type of the content 0 root dir, 1 dir, 2 file
			first_cluster: of the file or directory
			file_size: if the entry corresponds to a file it will contain the size of the file
		By value:
			int8_t with the result of the operation
					 1: the entry has been read
					: FAT32_file_dir is not initialized
					: FAT32_file_dir invalid information ( FAT32_file_dir perhaps corresponds to a file and so it does not have entries )
					: an error has ocured while reading the memory
					: invalid n_entry value, is out of directory boundary
*/
void FAT32_GetShortDirEntry(uint32_t * address,uint8_t * name,uint8_t * type, uint32_t * first_cluster,uint32_t * file_size){
	uint8_t i;
	uint8_t read_byte;
	uint32_t aux_doubleword;

	// first entry name is read
	i=0;
	while (i<MAX_NAME_SHORT_ENTRY+1){
		FAT32_ReadByteOffset(*address,&read_byte);
		(*address)++;
		name[i]=read_byte;
		i++;									
		if (i==8){
			// places the file extension dot '.'
			name[i]='.';
			i++;
		};//if
	};//while
	name[i]='\0';
		
	//then read the type of entry ( directory or file )
	FAT32_ReadByteOffset(*address,&read_byte);
	if ((read_byte&ATTR_DIRECTORY)>0){
		//is a directory
		*type=1;									
	}else if ((read_byte&ATTR_ARCHIVE)>0){
		//is a file
		*type=2;
	};//if
		
	//then read the HIGH part of the cluster where it starts
	*first_cluster=0;
	*address=(*address)+9;
	FAT32_ReadByteOffset(*address,&read_byte);
	aux_doubleword=(uint32_t)read_byte;
	*first_cluster=*first_cluster|(aux_doubleword<<16);
	(*address)++;
	FAT32_ReadByteOffset(*address,&read_byte);
	aux_doubleword=(uint32_t)read_byte;
	*first_cluster=*first_cluster|(aux_doubleword<<24);
												
	//then read the LOW part of the cluster where it starts
	*address=(*address)+5;
	FAT32_ReadByteOffset(*address,&read_byte);
	aux_doubleword=(uint32_t)read_byte;
	*first_cluster=*first_cluster|(aux_doubleword);
					
	(*address)++;
	FAT32_ReadByteOffset(*address,&read_byte);
	aux_doubleword=(uint32_t)read_byte;
	*first_cluster=*first_cluster|(aux_doubleword<<8);
													
	// if it is a file we also must read it's size
	if (*type==2){
		*file_size=0;
		
		(*address)++;
		FAT32_ReadByteOffset(*address,&read_byte);
		aux_doubleword=(uint32_t)read_byte;	
		*file_size=*file_size|(aux_doubleword);
		
		(*address)++;
		FAT32_ReadByteOffset(*address,&read_byte);
		aux_doubleword=(uint32_t)read_byte;
		*file_size=*file_size|(aux_doubleword<<8);
		
		(*address)++;
		FAT32_ReadByteOffset(*address,&read_byte);
		aux_doubleword=(uint32_t)read_byte;
		*file_size=*file_size|(aux_doubleword<<16);
		
		(*address)++;
		FAT32_ReadByteOffset(*address,&read_byte);
		aux_doubleword=(uint32_t)read_byte;
		*file_size=*file_size|(aux_doubleword<<24);
		
		(*address)++;// places read pointer at the begining of the next entry
	}else{
		(*address)=(*address)+4 ;// places read pointer at the begining of the next entry
	};//if
		
	// as preventive measure, if calculated *first_cluster value is 0 it is placed at the root
	// directory, some FAT32 files systems refer to the root cluster as cluster 0
	if ((*first_cluster==0)||(*first_cluster==FAT32_info.bpb_rootclus)){
		*first_cluster=FAT32_info.bpb_rootclus;
		type=0;
	};//if
	
};//FAT32_GetShortDirEntry



/*	Description:
	Returns the name, location (first cluster) and type of the n entry in a
	directory. Using this function we can move through the file system and
	read its content
	Recives:
		Obviously FAT32_file_dir structure must be initialized and contain
		directory data, not file data, otherway it wont find entries to read
		By value:
		entry_n: (where entry_n<FAT32_file_dir.number_of_files_dirs) the 
		number of the entry of the directory we want to read
	Returns:
		By reference:
			name: a string with the name of the file or directory
			type: with the type of the content 0 root dir, 1 dir, 2 file
			first_cluster: of the file or directory
			file_size: if the entry corresponds to a file it will contain the size of the file
			long_name: it's long name
		By value:
			int8_t with the result of the operation
					 1: the entry has been read
					-1: FAT32_file_dir is not initialized
					-2: FAT32_file_dir invalid information ( FAT32_file_dir perhaps corresponds to a file and so it does not have entries )
					-3: an error has ocured while reading the memory
					-4: invalid n_entry value, is out of directory boundary
*/
int8_t FAT32_GetDirElementN(uint16_t entry_n,uint8_t * name,uint8_t * type, uint32_t * first_cluster,uint32_t * file_size,uint8_t * long_name){
	int8_t ret;
	uint32_t aux_doubleword;
	uint8_t read_byte;
	
	// first we check that FAT32 has been initialized
	if  (FAT32_info.state!=1){
		ret=-1;
	}else{
	
		// cheks if FAT32_file_dir strucutre is loaded with file information
		if (FAT32_file_dir.type==2){
			ret=-2;
		}else{
		
			// checks if n_entry has a valid value
			if (entry_n>FAT32_file_dir.number_of_files_dirs){
				// invalid n_entry value, is out of directory boundary
				ret=-4;
			}else{
			
				// jumps to the first byte in the directory
				if (FAT32_file_dir.type==0){
					// is a root directory
					aux_doubleword=32;// jumps the first two 32 bytes of the volume entry
				}else{
					// is a standar directory
					aux_doubleword=0;
				};//if
				
				// first we initialize all the vars
				name[0]='\0';
				long_name[0]='\0';
				*type=0;
				*first_cluster=0;
				* file_size=0;
				
				do{
					// reads the first byte of the entry
					ret=FAT32_ReadByteOffset(aux_doubleword,&read_byte);
					if (ret<0){
						// an error has ocured while reading the memory
						ret=-3;
					}else{		
						// process the first byte of each entry to get more information
						switch (read_byte){
							case FREE_LAST_ENTRY:
								// we have arrived to the last entry, so we are in an invalid n_entry value, 
								// in other words entry_n is out of directory boundary
								ret=-4;
								break;
							case FREE_USED_ENTRY:
								aux_doubleword=aux_doubleword+32;//jumps the entry
								break;
							default:
								// reads the DIR_Attr byte of the entry
								ret=FAT32_ReadByteOffset(aux_doubleword+11,&read_byte);
								// once we are in a valid entry we check the type of entry 
								switch (read_byte){
									case VOLUME_ATTR:
										aux_doubleword=aux_doubleword+32;//jumps the entry
										break;
									
									case LONG_NAME_ATTR:
									case DIRECTORY_ATTR:
									case ARCHIVE_ATTR:
									
										// checks if we have arrived to the entry we were looking for
										if (entry_n!=1){
											// we haven't arrived yet to the entry we were looking for											
											if (read_byte==LONG_NAME_ATTR){
												// is a LONG_NAME_ATTR entry, so we read the first byte to get the number of entries wich compound this long name
												ret=FAT32_ReadByteOffset(aux_doubleword,&read_byte);
												read_byte=read_byte&0x3F;// removes 3 highest bits to get the number of "Long directory entries"
												aux_doubleword=aux_doubleword+(read_byte*32);//jump all the long entries
												// now is a DIRECTORY_ATTR, or a ARCHIVE_ATTR
												aux_doubleword=aux_doubleword+32;
											}else{
												// is a DIRECTORY_ATTR, or a ARCHIVE_ATTR entry, and we jump it
												// this case should not take place beacuse always should be a Long Name Entry before 
												// a DIRECTORY_ATTR or ARCHIVE_ATTR entry, but is here just in case this entries have no Long Entry
												aux_doubleword=aux_doubleword+32;
											};//if
										}else{
											
											// we have arrived to the entry we were looking for
											if ((read_byte==LONG_NAME_ATTR)&&(ret>=0)){
												ret=FAT32_GetLongDirEntry(&aux_doubleword,long_name);
												// behind the set of LONG ENTRIES, an ARCHIVE_ATTR or DIRECTORY_ATTR should be found
												ret=FAT32_ReadByteOffset(aux_doubleword+11,&read_byte);
											};//if
											
											if ( ( (read_byte==ARCHIVE_ATTR) || (read_byte==DIRECTORY_ATTR) )&&(ret>=0)){
												FAT32_GetShortDirEntry(&aux_doubleword,name,type,first_cluster,file_size);
											};//if
											
										};//if
										entry_n--;		
										break;
									default:
										break;
								};//switch
						};//switch
					};//if
				}while((entry_n!=0)&&(ret>0));
				
			};//if
		};//if
	};//if
	
	return ret;
	
};//FAT32_GetDirElementN



/*	Description:
	Loads the basic FAT32 information of the file or the directory wich is going to be read. This
	information is needed during read (and write) operations. This information includes the first
    cluster of the file or directory, the first sector, the name, the size (in case it is a file )
	, the actual read cluser, the actual read sector, the actual read byte ....
	Recives:
		By value:
			uint32_t with the number of fat entry (cluster), corresponding to the cluster where the file
			or directory begins. Reading its content will let us know if it is the last cluster or if it 
			continues in any other cluster.
			uint8_t: 0-Is root directory  1-Is a directory 2-Is a file
			uint8_t[MAX_NAME_SHORT_ENTRY]: with the name of the directory or file whose information is going 
			to be loaded
			uint32_t: with the size of the file ( only usefull if it is a file in other cases must be 0 )
	Returns:
		By value:
			 1: "FAT32_file_dir" info has been loaded initialized correctly
			-1: any error has ocurred and has not been possible ot initialize the FAT32_LoadFileDirInfo structure
			-2: any invalid n_fat_entry_cluster has been recived
			-3: any invalid type has been recived
			-4: error when reading the directory entries
*/
int8_t FAT32_LoadDirElement(uint32_t n_fat_entry_cluster,uint8_t type, uint8_t * name, uint32_t size){
	int8_t ret;
	uint32_t aux_double_word;

	
	// first we check if FAT32_file_dir has been initialized
	if (FAT32_info.state==1){
		
		// as preventive measure if n_fat_entry is 0 it is placed at the root directory, some 
		// FAT32 files systems refer to the root cluster as cluster 0
		if ((n_fat_entry_cluster==0)||(n_fat_entry_cluster==FAT32_info.bpb_rootclus)){
			n_fat_entry_cluster=FAT32_info.bpb_rootclus;
			type=0;
		};//if
		
		// reads the cluster entry related to n_fat_entry_cluster, and checks that n_fat_entry
		// corresponds to a correct cluster
		if (FAT32_IsValidCluster(n_fat_entry_cluster,&aux_double_word)>=0){				
			FAT32_file_dir.previous_first_fat_entry=FAT32_file_dir.first_fat_entry;// stores the current first_fat_entry before modifiying it wit the new value
			FAT32_file_dir.first_fat_entry=n_fat_entry_cluster;// NUMBER of the first FAT entry of the directory or file. With this number and the information contained in FAT32_info structure can be read the content of the first FAT entry of the file or directory wich is being read
						
			FAT32_file_dir.first_sector= FAT32_ClusterFirstSector(n_fat_entry_cluster);// number of sector where starts the first cluster of the file or directory
			FAT32_file_dir.number_of_read_cluster=n_fat_entry_cluster;// number of cluster in the memory wich is being read
			FAT32_file_dir.number_of_read_sector= FAT32_file_dir.first_sector;// number of sector in the memory wich is being read
			FAT32_file_dir.number_of_read_byte=0;// number of byte inside the sector wich is being read
			FAT32_file_dir.last_read_byte_offset=0;// last read byte in the file, it is very usefull to know if is necessary to recalculate the adresses of cluster, sectors or bytes
			FAT32_file_dir.size=size;
			
			//stores the old name into the previous_name array and, copies the new name into the FAT32_file_dir structre
			ret=0;
			while (ret<MAX_NAME_SHORT_ENTRY){
				FAT32_file_dir.previous_name[ret]=FAT32_file_dir.name[ret];// copies the current in the old array
				FAT32_file_dir.name[ret]=name[ret];// copies the new name in the array
				ret++;
			};//while
			FAT32_file_dir.previous_name[ret]='\0';
			FAT32_file_dir.name[ret]='\0';
			ret=1;
			
			// sets the type of FAT32_file_dir (0-Root dir 1-Dir 2-File) and the number of entries if it is a directory
			FAT32_file_dir.type=type;
			switch (type){
				case 0:
				case 1:
					// if it's type corresponds to a directory or root directory it's number of entries is loaded
					if (FAT32_GetNumberDirElements(&FAT32_file_dir.number_of_files_dirs)<0){
						// error when reading the directory entries
						ret=-4;
					};//if
					break;
				case 2:
					// it is a file and does not contain entries
					FAT32_file_dir.number_of_files_dirs=0;
					break;
				default:
					// any invalid type has been recived
					ret=-3;
					break;
			};//switch
			
		}else{
			// any invalid cluster ( invalid n_fat_entry_cluster ) has been recived
			ret=-2;
		};//if
	}else{
		// FAT32_info structure has not been initialized yet
		ret=-1;	
	};//if
	
	return ret;
	
};//FAT32_LoadDirElement



/*	Description:
	Restores the basic FAT32 current information with the one stored into the PREVIOUS vars, so exectuing this 
	function we can go back to the directory on wich we were before enterning the current directory or file. In
	case of directories it is not usefull at all, because all them contain the ".." entry wich allow us to go back
	to the precedent directory, but in case of files it is very usefull because, once we enter in a file,
	it allows us to go back to the dir, by restoring the FAT32 information  with the directotry where we were 
	before entering on it.
	Recives:
	It only requires that the previous_fat_entry and previous_name had been previously intialized
	with the data of the directory on wich we where before entering to the current fil or directory
	Returns:
		By value:
		The result of the FAT32_LoadDirElement operation ( see FAT32_RestorePreviousDirElement operation codes )
*/
int8_t FAT32_RestorePreviousDirElement(){
	int8_t ret;
	
	// check the value of the previous_first_fat_entry to see if corresponds to a stanadard dir or to a root directory
	if ( (FAT32_file_dir.previous_first_fat_entry==0)
		|| (FAT32_file_dir.previous_first_fat_entry==FAT32_info.bpb_rootclus) ){
		ret=FAT32_LoadDirElement(FAT32_file_dir.previous_first_fat_entry,0,FAT32_file_dir.previous_name, 0);
	}else{
		ret=FAT32_LoadDirElement(FAT32_file_dir.previous_first_fat_entry,1,FAT32_file_dir.previous_name, 0);
	};//if

	return ret;
	
};//FAT32_RestorePreviousDirElement



/*	Description:
	Reads the next byte in the file or directory, it uses the "FAT32_file_dir.last_read_byte_offset"
	to store the @ of last read byte, so it will increase it's value each time it reads a byte
	*NOTE: this function is more efficient than FAT32_ReadByteOffset beacuse it executes less
	arithmetic operations
	Recives:
		By reference:
			uint8_t: the byte where the result of read operation will be stored
	Returns:
		By reference:
			uint8_t: the read byte at offset position
		By value:
			 1: the byte has been read correctly
			-1: FAT32_info structure has not been initialized yet
			-2: any error has ocurred and has not been possible to read the byte
			-3: the byte is in an invalid cluster
*/
int8_t FAT32_ReadByte(uint8_t * read_byte){
	int8_t ret;
	uint32_t aux_double_word;
	
	ret=1;

	// first we check that FAT32 has been initialized,
	if (FAT32_info.state!=1){
		ret=-1;
	}else{
		// byte is read 
		ret=SDMMC_ReadByte(FAT32_file_dir.number_of_read_sector,FAT32_file_dir.number_of_read_byte,read_byte);
		if (ret<=0){
			// an error has occured while reading the byte			
			ret=-1;
		}else{
			// the byte has been succesfully read 
			FAT32_file_dir.number_of_read_byte++;
			FAT32_file_dir.last_read_byte_offset++;
			// checks if we have reached the end of the sector
			if (FAT32_file_dir.number_of_read_byte==FAT32_info.bpb_bytspersec){
				FAT32_file_dir.number_of_read_byte=0;			
				// calculates if we have reached the end of the cluster to see if we have to read the FAT to
				// get next cluster and its first sector, or if we only have to incresase in one the sector address
				aux_double_word=FAT32_file_dir.last_read_byte_offset/FAT32_info.bpb_bytspersec;
				if (aux_double_word%FAT32_info.bpb_secperclus==0){
					// we have reached the end of the cluster and we have to calculate next one, so  FAT32_IsValidCluster 
					// is called to see if the actual cluster is the last one or there are more clusters behind it
					if (FAT32_IsValidCluster(FAT32_file_dir.number_of_read_cluster,&aux_double_word)>=0){
						FAT32_file_dir.number_of_read_cluster=aux_double_word;
						FAT32_file_dir.number_of_read_sector=FAT32_ClusterFirstSector(FAT32_file_dir.number_of_read_cluster);
					}else{
						ret=-3;
					};//if
				}else{
					// we haven't reached yet the end of the cluster so we only have to increase in one the number of sector
					FAT32_file_dir.number_of_read_sector++;
				};//if
			};//if
		};//if
	};//if
	
	return ret;

};//FAT32_ReadByte



/*	Description:
	Reads the byte placed at offset bytes from the first sector ( of the first cluster )
	of the file or directory. It allows to acces  the file or the directory as a lineal
	secuence of bytes, avoiding to treat with clusters and sectors.
	*NOTE: it is an easy to use but not the most eficient way to acces the memory beacuse 
	each read acces implies some arithmetic operations. Secuencial mode or block read are 
	recommended
	Recives:
		By value:
			uint32_t with the offset of bytes we want to read from the begining of the
			file or directory
		By reference:
			uint8_t: the byte where the result of read operation will be stored
	Returns:
		By reference:
			uint8_t: the read byte at offset position
		By value:
			 1: the byte has been read correctly
			-1: FAT32_info structure has not been initialized yet
			-2: any error has ocurred and has not been possible to read the byte
			-3: the byte is in an invalid cluster
*/
int8_t FAT32_ReadByteOffset(uint32_t offset,uint8_t * read_byte){
	int8_t ret;
	uint32_t aux_double_word;
	uint32_t aux_double_word2;
	
	ret=1;

	// first we check that FAT32 has been initialized, and the cluster is a valid cluster
	if (FAT32_info.state==1){
		
		// gets the number of clusters behind the offset byte
		aux_double_word=offset /  (FAT32_info.bytes_per_cluster);
		aux_double_word2=FAT32_file_dir.last_read_byte_offset / (FAT32_info.bytes_per_cluster);
		
		// OPTIMIZATION:if the number of clusters behind the offset bytes is diferent to the number of 
		// clustersbehind the byte read last time, it means that the cluster to read is diferent, and we
		// have to calculate it's address again. But if it is the same, it means we calculated it previously
		// and it is no necessary to recalculate its cluster address, because it is already stored 
		// into "FAT32_file_dir.number_of_read_cluster" structure
		if (aux_double_word!=aux_double_word2){
		
			// OPTIMIZATION: if the number of clusters behind the "offset" byte is larger than the number of 
			// clusters behind the byte read last time we don't have to go back to the begining of the file counting 
			// from the first cluster, we can continue calculating from last read cluster, it avoids recalculating 
			// all previously recalculated clusters each time we read in a new cluster that is behind the last 
			// read one. In the other side, if the number of clusters behind the "offset" byte is lower than the number
			// of previously calculated cluster we recalculate again its cluster from the begining
			if (aux_double_word>aux_double_word2){
				// the number of cluster jumps decresases from (aux_double_word) to (aux_double_word-aux_double_word2)
				// what is faster than excuete aux_double_word again
				aux_double_word=aux_double_word-aux_double_word2;
			}else{
				// the number of cluster jumps is aux_double_word
				FAT32_file_dir.number_of_read_cluster=FAT32_file_dir.first_fat_entry;
				
			};//if
			
			// looks for the "offset byte" cluster 
			while((aux_double_word!=0)&&(ret>0)){
					
				// reads the cluster entry and checks it, if it is ok aux_double_word2 will contain the entry
				if (FAT32_IsValidCluster(FAT32_file_dir.number_of_read_cluster,&aux_double_word2)>=0){				
					FAT32_file_dir.number_of_read_cluster=aux_double_word2;// if entry is valid we jump to it
					aux_double_word--;// the var wich controls the numer of remaining jumps is decreased
				}else{
					// we are in an invalid cluster
					FAT32_file_dir.number_of_read_cluster=FAT32_file_dir.first_fat_entry;// reset read cluster to the first cluster of the file or directory
					ret=-3;
				};//if
				
			};//while
			
		};//if
		
		if (ret==1){
			// gets the sector of the "offset byte" 
			FAT32_file_dir.number_of_read_sector=FAT32_ClusterFirstSector(FAT32_file_dir.number_of_read_cluster);
			
			aux_double_word=offset % FAT32_info.bytes_per_cluster;
			FAT32_file_dir.number_of_read_sector=FAT32_file_dir.number_of_read_sector + (aux_double_word / FAT32_info.bpb_bytspersec);
			
			// gets the number of byte inside the sector wich corresponds to the "offset byte"
			FAT32_file_dir.number_of_read_byte=offset%FAT32_info.bpb_bytspersec;
		
			// once we have its location byte is read 
			ret=SDMMC_ReadByte(FAT32_file_dir.number_of_read_sector,FAT32_file_dir.number_of_read_byte,read_byte);
			if (ret!=1){
				// an error has occured while reading the byte
				ret=-2;
			}else{
				// the byte has been read and FAT32_file_dir.last_read_byte_offset is updated with las read offset
				FAT32_file_dir.last_read_byte_offset=offset;
			};//if
		
		};//if
	
	}else{
		// FAT32_fil_dir has not been initi
		ret=-1;
	};//if
	
	return ret;
	
};//FAT32_ReadByteOffset


/*	Description:
	Reads the media type bye (offset 21 byte) to check if the sector specified in the 
	global structure FAT32_info.partition_first_sector corresponds to a valid first 
	FAT32 sector. To do	that it reads the Media Type byte ( offset 21 byte ) and checks
	if it contains a valid value, in this case the sector is considered to be a valid 
	first FAT32 sector in other cases it is not considereded a valid sector.
	Recives:	
	Returns:	
	By value:
		 1: It is a valid FAT32 first sector ( the sector with a BPB )
		-1: It is not a valid FAT32 first sector
*/
int8_t FAT32_IsValidFirstSector(){
	int8_t ret;
	uint8_t aux_byte;
	
	// read the bpb_media_type position to check if it has a valid media type value
	ret=SDMMC_ReadByte(FAT32_info.partition_first_sector,21,&aux_byte);
	if (ret==1){
		
		switch (aux_byte){
			case 0xF0://removable media type
			case 0xF8://fixed media type
			case 0xF9://other valid media types
			case 0xFA:
			case 0xFB:
			case 0xFC:
			case 0xFD:
			case 0xFE:
			case 0xFF:
				ret=1;
				break;
			default:
				ret=-1;
				break;
		}//switch
	
	}//if

	return ret;

}//FAT32_IsValidFirstSector



/*	Description:
	Loads the FAT32 basic information from SD_MMC card (BPB and root directory data) to 
	operate with the file system
	Recives:
	Returns:
	By value:
		 1: FAT32_info has been loaded into the memory structure
		-1: any error has ocurred and has not been possible ot initialize the FAT32 structure
*/
int8_t FAT32_Initialize(){
	int8_t ret;
	uint8_t aux_byte;
	uint8_t aux_byte2;
	uint8_t aux_byte3;
	uint32_t aux_double_word;
	uint32_t aux_double_word2;
	uint32_t bpb_totsec32=0;// FAT32 BPB_TotSec32 used only locally to check if our system is FAT32
	uint8_t aux_string[MAX_NAME_SHORT_ENTRY+1];
	
	
	// SD MMC cadrd low level initialization
	ret=SDMMC_InitializeCard();

	// Start checking if it is a "big floppy" unit or it has an MBR with a partition structure,
	// the objective is to locate the first sector of a valid FAT32 partition
	if (ret==1){
		
		FAT32_info.partition_first_sector=0;
		if (FAT32_IsValidFirstSector()==-1){
			
			// sector 0 is not a valid "first FAT32 sector" ( is not a "bigfloppy") then we
			// check if it corresponds to a MBR sector, so check each of the partition entries
			aux_byte2=0;// flag to control the while
			aux_byte3=0;// to count the number of processed entries
			while ( (ret!=-1) && (aux_byte2==0) && (aux_byte3<MAX_MBR_PRIMARY_PARTITIONS) ){
				
				ret=SDMMC_ReadByte(0,(uint16_t) (MBR_FIRST_PARTITION_ENTRY_OFFSET + (MBR_PARTITION_ENTRY_LENGTH*aux_byte3)),&aux_byte);
				if ( (ret==1) && ((aux_byte==0x80)||(aux_byte==0x00)) ){
					
					// seems to be a valid entry so try to read the number of sector where the partition begins
					ret=ret&SDMMC_ReadByte(0,(uint16_t) (MBR_FIRST_PARTITION_ENTRY_OFFSET + (MBR_PARTITION_ENTRY_LENGTH*aux_byte3)+8),&aux_byte);
					aux_double_word2=aux_byte;
					ret=ret&SDMMC_ReadByte(0,(uint16_t) (MBR_FIRST_PARTITION_ENTRY_OFFSET + (MBR_PARTITION_ENTRY_LENGTH*aux_byte3)+9),&aux_byte);
					aux_double_word=aux_byte;
					aux_double_word2=aux_double_word2|(aux_double_word<<8);
					ret=ret&SDMMC_ReadByte(0,(uint16_t) (MBR_FIRST_PARTITION_ENTRY_OFFSET + (MBR_PARTITION_ENTRY_LENGTH*aux_byte3)+10),&aux_byte);
					aux_double_word=aux_byte;
					aux_double_word2=aux_double_word2|(aux_double_word<<16);
					ret=ret&SDMMC_ReadByte(0,(uint16_t) (MBR_FIRST_PARTITION_ENTRY_OFFSET + (MBR_PARTITION_ENTRY_LENGTH*aux_byte3)+11),&aux_byte);
					aux_double_word=aux_byte;
					aux_double_word2=aux_double_word2|(aux_double_word<<24);
					FAT32_info.partition_first_sector=aux_double_word2;
					
					if (FAT32_IsValidFirstSector()==-1){
						// the FAT32_info.partition_first_sector processed sector is NOT a valid first FAT32 sector
						aux_byte3++;
					}else{
						// the FAT32_info.partition_first_sector processed sector is a valid first FAT32 sector
						aux_byte2=1;// stop the bucle
						ret=1;// return OK
					}//if
					
				}else{
					// not a valid MBR, leave the bucle
					aux_byte2=1;// stop the bucle
					ret=-1;// return NOT OK
				}//if
				
			}//while
			
		}else{
			
			// sector 0 is a valid FAT32 sector
			aux_byte2=1;// stop the bucle
			ret=1;// return OK

		}//if
		
	}//if

	if (ret==1){		
		// Load BPB_BytesPerSec
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,11,&aux_byte);
		aux_double_word2=aux_byte;
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,12,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<8);
		FAT32_info.bpb_bytspersec=(uint16_t)aux_double_word2;		
	};//if
	
	if (ret==1){
		// Load BPB_SecPerClus 
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,13,&aux_byte);
		FAT32_info.bpb_secperclus=aux_byte;
	};//if

	if (ret==1){
		// Load BPB_ResvdSecCnt
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,14,&aux_byte);
		aux_double_word2=aux_byte;
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,15,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<8);
		FAT32_info.bpb_resvdseccnt=(uint16_t)aux_double_word2;
	};//if

	if (ret==1){
		// Load BPB_NumFATs
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,16,&aux_byte);
		FAT32_info.bpb_numfats=aux_byte;
	};//if
		
	if (ret==1){
		// Load BPB_TotSec32 (for local use!!)
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,32,&aux_byte);
		aux_double_word2=aux_byte;
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,33,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<8);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,34,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<16);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,35,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<24);
		bpb_totsec32=aux_double_word2;
	};//if

	if (ret==1){
		// Load BPB_FatSz32
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,36,&aux_byte);
		aux_double_word2=aux_byte;
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,37,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<8);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,38,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<16);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,39,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<24);
		FAT32_info.bpb_fatsz32=aux_double_word2;
	};//if
	
	if (ret==1){
		// Load BPB_RootClus
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,44,&aux_byte);
		aux_double_word2=aux_byte;
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,45,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<8);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,46,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<16);
		ret=ret&SDMMC_ReadByte(FAT32_info.partition_first_sector,47,&aux_byte);
		aux_double_word=aux_byte;
		aux_double_word2=aux_double_word2|(aux_double_word<<24);
		FAT32_info.bpb_rootclus=aux_double_word2;		
	};//if
	
	//if (ret==1){		
	// Load BS_VolLab
	//	for(aux_double_word=0;aux_double_word<11;aux_double_word++){
	//		ret=ret&SDMMC_ReadByte(0,71+aux_double_word,&aux_byte);
	//		FAT32_info.bs_vollab[aux_double_word]=aux_byte;
	//	};//for
	//	FAT32_info.bs_vollab[12]='\0';// end of string symbol is placed to avoid problems when printing the string
	//};//if

	// Cheks if it is FAT32
	// DataSec = BPB_TotSec32 – (BPB_ResvdSecCnt + (BPB_NumFATs * BPB_FATSz32));
	// CountofClusters = DataSec / BPB_SecPerClus;
	// if(CountofClusters >= 65525){
	//     !!!Volume is FAT32!!!
	// }
	aux_double_word = (uint32_t) (bpb_totsec32 - FAT32_info.bpb_resvdseccnt - (FAT32_info.bpb_numfats * FAT32_info.bpb_fatsz32));
	aux_double_word2 = (uint32_t)(aux_double_word / FAT32_info.bpb_secperclus);
	if ((ret!=1)||(aux_double_word2<65525)){
		// If the card is not a FAT32 card we reset the structure
		FAT32_Reset();
		ret=-1;
	}else{
		FAT32_info.bytes_per_cluster=FAT32_info.bpb_secperclus* FAT32_info.bpb_bytspersec;// Precalculated number of bytes per cluster ( bpb_secperclus * bpb_secperclus ) - 4 bytes
		FAT32_info.state=1;// FAT32 basic information is now Initialized
		// Number of the first sector of the data region:
		// FirstDataRegionSector = First_FAT32_sector + BPB_Resvd_SecCnt + (BPB_NumFats x BPB_FatSz32)
		FAT32_info.first_data_region_sector=(uint32_t)(FAT32_info.partition_first_sector+FAT32_info.bpb_resvdseccnt+(FAT32_info.bpb_numfats * FAT32_info.bpb_fatsz32));
		
		// Read the name of the FAT32 volume in the Root Directory
		aux_byte=0;
		//while ((aux_byte2!='\0')&&(aux_byte<MAX_NAME_SHORT_ENTRY)&&(ret==1)){
		while ((aux_byte<MAX_NAME_SHORT_ENTRY)&&(ret==1)){
			ret=ret&SDMMC_ReadByte(FAT32_info.first_data_region_sector,aux_byte,&aux_byte2);
			aux_string[aux_byte]=aux_byte2;
			aux_byte++;
		};//for
		aux_string[aux_byte]='\0';// just in case 
		
		if (ret==1){
			// Once we have all information, FAT32_file_dir root dir (type is 0) info is initialized 
			FAT32_LoadDirElement(FAT32_info.bpb_rootclus,0,aux_string,0);
		}else{
			// If the card is not a FAT32 card we reset the structure
			FAT32_Reset();
			ret=-1;
		};//if
		
	};//if
	
	return ret;

};//FAT32_Initialize



/*	Description:
	Tests routines, which lets check if the FAT32 works properly or not
	Recives:
		step: with the test routine to execute
	Returns:
	  By value:
	  By reference:
 */
void FAT32_Test(int8_t step){
	uint8_t ui8_aux;
	int8_t ret;
	uint16_t ui16_aux;
	
	static uint32_t ui32_aux=0;
	static uint32_t ui32_aux3=0;
	static uint32_t ui32_aux4=0;


	uint8_t aux_name[MAX_NAME_SHORT_ENTRY+1+1];
	uint8_t aux_name2[MAX_LONG_NAME+1+1];
	
	switch (step){
		case 0:
			ui32_aux++;
			UART0_PrintString("En:");
			UART0_PrintUint(ui32_aux);
			UART0_ForceBufferTransmission();
			break;
		case 1:
			ui32_aux--;
			UART0_PrintString("En:");
			UART0_PrintUint(ui32_aux);
			UART0_ForceBufferTransmission();
			break;
		case 2:
			if (FAT32_GetNumberDirElements(&ui16_aux)>=0){
				UART0_PrintString("Total:");
				UART0_PrintUint(ui16_aux);
				UART0_PrintString("\r\n");
				UART0_ForceBufferTransmission();
				ret=0;
				while ((ret>=0)&&(ui16_aux>0)){
					ret=FAT32_GetDirElementN(ui16_aux,aux_name,&ui8_aux,&ui32_aux3,&ui32_aux4,aux_name2);
					if (ret>=0){
						UART0_PrintUint(ui16_aux);
						
						UART0_PrintString(":Ok");
						UART0_PrintString(" Nme:");
						UART0_PrintString(aux_name);
						UART0_ForceBufferTransmission();
				
						UART0_PrintString(" LngNme:");
						UART0_PrintString(aux_name2);
						UART0_ForceBufferTransmission();
				
						UART0_PrintString(" Clus:");
						UART0_PrintUint(ui32_aux3);
						UART0_ForceBufferTransmission();
		
						if (ui8_aux==1){
							UART0_PrintString(" is Dir");
						};//if
						if (ui8_aux==2){
							UART0_PrintString(" is File");
							
							UART0_PrintString(" Size:");
							UART0_PrintUint(ui32_aux4);
							UART0_ForceBufferTransmission();
							
						};//if	
						UART0_PrintString("\r\n");
						UART0_ForceBufferTransmission();
						ui16_aux--;
					};//if
				};//while
			}else{
				UART0_PrintString("Err");
				UART0_ForceBufferTransmission();
			};//if
			break;
		case 3:
			ret=FAT32_GetDirElementN((uint16_t)ui32_aux,aux_name,&ui8_aux,&ui32_aux3,&ui32_aux4,aux_name2);
			if (ret>=0){
				UART0_PrintString("Entr:");
				UART0_PrintUint(ui32_aux);
				UART0_PrintString("Ok");
				UART0_ForceBufferTransmission();
	
				UART0_PrintString(" Nme:");
				UART0_PrintString(aux_name);
				UART0_ForceBufferTransmission();
		
				UART0_PrintString(" LngNme:");
				UART0_PrintString(aux_name2);
				UART0_ForceBufferTransmission();
				
				UART0_PrintString(" Clus:");
				UART0_PrintUint(ui32_aux3);
				UART0_ForceBufferTransmission();

				if (ui8_aux==1){
					UART0_PrintString(" is Dir");
				};//if
				
				if (ui8_aux==2){
					UART0_PrintString(" is File");
						
					UART0_PrintString(" Size:");
					UART0_PrintUint(ui32_aux4);
					UART0_ForceBufferTransmission();
							
				};//if	
				UART0_PrintString("\r\n");
				UART0_ForceBufferTransmission();
				
				ret=FAT32_LoadDirElement(ui32_aux3,ui8_aux,aux_name,ui32_aux4);
				if (ret<0){
					UART0_PrintUint(10+ret);
					UART0_PrintString("EntLoadErr\r\n");
					UART0_ForceBufferTransmission();
				}else{
					UART0_PrintString("Loaded!\r\n");
					UART0_ForceBufferTransmission();
				};//if
			
			}else{
				UART0_PrintString("EntryErr\r\n");
				UART0_ForceBufferTransmission();
			};//if
		
			break;
		case 4:
			ret=FAT32_RestorePreviousDirElement();
			if (ret>=0){
				UART0_PrintString("Restored\r\n");
				UART0_ForceBufferTransmission();
			}else{
				UART0_PrintString("RestoreError\r\n");
				UART0_ForceBufferTransmission();
			};//if
			break;
		case 5:
			if (FAT32_file_dir.type!=2){
				UART0_PrintString("Not a file!\r\n");
				UART0_ForceBufferTransmission();
			}else{
				UART0_PrintString("Reading ");
				UART0_PrintString(FAT32_file_dir.name);
				UART0_PrintString(" ");
				UART0_PrintUint(FAT32_file_dir.size);
				UART0_PrintString(" bytes\r\n");
				UART0_ForceBufferTransmission();		
				ui32_aux4=0;
				ret=0;
				while ((ret>=0)&&(ui32_aux4<FAT32_file_dir.size)){
					ret=FAT32_ReadByteOffset(ui32_aux4,&ui8_aux);
					UART0_PrintByte(ui8_aux);
					UART0_ForceBufferTransmission();
					ui32_aux4++;
				};//hwile
			};//while
			break;
		default:
			break;
	};//switch

};//SDMMC_Test



