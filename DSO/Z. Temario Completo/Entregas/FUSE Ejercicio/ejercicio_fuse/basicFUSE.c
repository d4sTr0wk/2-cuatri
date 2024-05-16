/*
 * FUSE: Filesystem in Userspace
 * DSO 2014
 * Ejemplo para montar un libro de poesía como sistema de ficheros
 * Cada capítulo del libro será un fichero diferente
 * 
 * Compilarlo con make
 *  
 * uso:  basicFUSE [opciones FUSE] fichero_inicial punto_de_montaje
 * 
 *  ./basicFUSE proverbiosycatares.txt punto_montaje
*/
#define FUSE_USE_VERSION 26

#include "basicFUSE_lib.h"

#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/*
    Para usar los datos pasados a FUSE usar en las funciones:
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
*/

static int mi_getattr(const char *path, struct stat *stbuf){
	
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	int i;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));

	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;  // S_IFDIR directory.
		stbuf->st_nlink = 2;  // The field “st_nlink” specifies the number of hardlinks.
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;

		stbuf->st_size = 1024;
		stbuf->st_blocks = 2;

	} else if(strcmp(path + 1, "BIG") == 0){
		// Es un "/BIG"
		stbuf->st_mode = S_IFDIR | 0755;  // S_IFDIR directory.
		stbuf->st_nlink = 2;  // The field “st_nlink” specifies the number of hardlinks.
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;

		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;

		stbuf->st_size = 1024;
		stbuf->st_blocks = 2;

	} else if((strncmp(path + 1, "BIG", 3) == 0) && ((i = buscar_fichero(path + 4, mis_datos)) >= 0)){
		// Es un "/BIG/X..."
		stbuf->st_mode = S_IFREG | 0444;  // S_IFREG regular file.
		stbuf->st_nlink = 1;  // The field “st_nlink” specifies the number of hardlinks.
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;
		
		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size / 512 + (stbuf->st_size % 512) ? 1 : 0;		
	} else if(strcmp(path + 1, "little") == 0){
		stbuf->st_mode = S_IFDIR | 0755;  // S_IFDIR directory.
		stbuf->st_nlink = 2;  // The field “st_nlink” specifies the number of hardlinks.
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;

		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;

		stbuf->st_size = 1024;
		stbuf->st_blocks = 2;

	} else if((i = buscar_fichero(path + 7, mis_datos)) >= 0){
		stbuf->st_mode = S_IFREG | 0444;  // S_IFREG regular file.
		stbuf->st_nlink = 1;  // The field “st_nlink” specifies the number of hardlinks.
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;
		
		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size / 512 + (stbuf->st_size % 512) ? 1 : 0;	
	} else if((i = buscar_fichero(path, mis_datos)) >= 0) {
		stbuf->st_mode = S_IFREG | 0444;  // S_IFREG regular file.
		stbuf->st_nlink = 1;  // The field “st_nlink” specifies the number of hardlinks.
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;
		
		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size / 512 + (stbuf->st_size % 512) ? 1 : 0;
	} else {
		res = -ENOENT;
	}
	return res;
}

/*****************************************************************************/

static int mi_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	int i;
	int pos;
	const char *aux;
	pos = buscar_fichero(path , mis_datos);

	(void) offset;
	(void) fi;

	if(strcmp(path, "/") == 0){
		if(filler(buf, "." , NULL, 0) != 0)  return -ENOMEM;
		if(filler(buf, "..", NULL, 0) != 0 ) return -ENOMEM;

		if(filler(buf, "BIG" , NULL, 0) != 0) return -ENOMEM;
		if(filler(buf, "little", NULL, 0) != 0) return -ENOMEM;

		for(i = 0; i < mis_datos->numero_ficheros; i++){
			if(filler(buf, mis_datos->nombre_ficheros[i], NULL, 0) != 0){
				return -ENOMEM;
			}
		}
	} else if(strcmp(path + 1, "BIG") == 0) {
		if(filler(buf, "." , NULL, 0) != 0) return -ENOMEM;
		if(filler(buf, "..", NULL, 0) != 0) return -ENOMEM;

		// 					EN OBRAS
		for(i = 0; i < mis_datos->numero_ficheros; i++){
			if(strlen(mis_datos->contenido_ficheros[i]) > 256){
				if(filler(buf, mis_datos->nombre_ficheros[i], NULL, 0) != 0){
					return -ENOMEM;
				}				
			}
		}
		/*
		for(i = 0; i < mis_datos->numero_ficheros; i++){
			if(strlen(mis_datos->contenido_ficheros[i]) > 256){
				if(filler(buf, mis_datos->nombre_ficheros[i], NULL, 0) != 0){
					return -ENOMEM;
				}						
			}
		}
		*/
	} else if(strcmp(path + 1, "little") == 0) {
		if(filler(buf, "." , NULL, 0) != 0) return -ENOMEM;
		if(filler(buf, "..", NULL, 0) != 0) return -ENOMEM;
		// 					EN OBRAS
		for(i = 0; i < mis_datos->numero_ficheros; i++){
			if(strlen(mis_datos->contenido_ficheros[i]) <= 256){
				if(filler(buf, mis_datos->nombre_ficheros[i], NULL, 0) != 0){
					return -ENOMEM;
				}	
			}
		}
		/*
		for(i = 0; i < mis_datos->numero_ficheros; i++){
			if(strlen(mis_datos->contenido_ficheros[i]) <= 256){
				mi_link(mis_datos->nombre_ficheros[i], aux);
				if(filler(buf, aux, NULL, 0) != 0){
					return -ENOMEM;
				}	
			}
		}
		*/
	}
	return 0;
}

/*****************************************************************************/

static int mi_open(const char *path, struct fuse_file_info *fi){
	
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	int pos;
	/*
	pos = buscar_fichero(path , mis_datos);

	if(pos >= 0){
		fi->fh = pos;
	}
	*/

	// Me llega "/X..."
	if(strcmp(path, "/") == 0) {
		pos = buscar_fichero(path , mis_datos);	
		if(strcmp(path + 1, mis_datos->nombre_ficheros[pos]) != 0){
			return -ENOMEM;
		}
	// Me llega "/BIG/X..."
	} else if((strncmp(path + 1, "BIG", 3) == 0) && (buscar_fichero(path + 4,mis_datos) >= 0)){
		pos = buscar_fichero(path + 4, mis_datos);
		if(strcmp(path + 4, mis_datos->nombre_ficheros[pos]) != 0){
			return -ENOMEM;
		}
	// Me llega "/little/X..."
	} else if(strncmp(path + 1, "little", 6) == 0){
		pos = buscar_fichero(path + 7, mis_datos);
		if(strcmp(path + 7, mis_datos->nombre_ficheros[pos]) != 0){
			return -ENOMEM;
		}
	}

	if((fi->flags & 3) != O_RDONLY){
		return -EACCES;
	}
	
	return 0;
}

/*****************************************************************************/

//   ME LA DAN HECHA.
// int buscar_fichero(const char *path, struct structura_mis_datos *mis_datos);
// Devuelve la posición del capítulo en la estructura mis_datos y devuelve -1 en caso de no ser encontrado.

/*****************************************************************************/

static int mi_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	size_t len;
	(void) fi;

	int pos;
	pos = buscar_fichero(path , mis_datos);
	const char *contenido = mis_datos->contenido_ficheros[pos];
	len = strlen(contenido);

	if(strcmp(path+1, mis_datos->nombre_ficheros[pos]) != 0){
		return -ENOENT;
	}

	len = strlen(mis_datos->contenido_ficheros[pos]);
	if(offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, mis_datos->contenido_ficheros[pos] + offset, size);
	} else{
		size = 0;
	}

	return size;
}

/******************************* operaciones FUSE ******************************/

static struct fuse_operations basic_oper = {
	.getattr	= mi_getattr,
	.readdir	= mi_readdir,
	.open		= mi_open,
	.read		= mi_read,
};

/*****************************************************************************/

int main(int argc, char *argv[]){
	struct structura_mis_datos *mis_datos;
	
	mis_datos = malloc(sizeof(struct structura_mis_datos));
	
	// análisis parámetros de entrada
	if((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-')) error_parametros();

	mis_datos->fichero_inicial = strdup(argv[argc-2]); // fichero donde están los capítulos
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    leer_fichero(mis_datos);
    
	/*  
    int i;
    for(i = 0; i < mis_datos->numero_ficheros; i++){
		printf("----->  %s\n", mis_datos->nombre_ficheros[i]);
		printf("%s",mis_datos->contenido_ficheros[i]);
	}
	exit(0);
	*/
	
	return fuse_main(argc, argv, &basic_oper, mis_datos);
}
