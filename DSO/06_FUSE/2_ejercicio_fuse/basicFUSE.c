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
 * 
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
 *  Para usar los datos pasados a FUSE usar en las funciones:
 * 
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
 * 
 * */

/***********************************
 * */
static int mi_getattr(const char *path, struct stat *stbuf)
{
	/* completar */
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	
	int i;
	
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) { // directorio raíz
		stbuf->st_mode = S_IFDIR | 0755; // directorio con permisos de lectura y ejecución
		stbuf->st_nlink = 2; // directorio con dos enlaces
		stbuf->st_uid = mis_datos->st_uid; // usuario propietario
		stbuf->st_gid = mis_datos->st_gid; // grupo propietario
		
		stbuf->st_atime = mis_datos->st_atime; // último acceso
		stbuf->st_mtime = mis_datos->st_mtime; // última modificación
		stbuf->st_ctime = mis_datos->st_ctime; // última modificación de los metadatos
		stbuf->st_size = 1024; // tamaño del directorio
		stbuf->st_blocks = 2; // bloques ocupados por el directorio
	} else if ((i= buscar_fichero(path, mis_datos)) >= 0) { // fichero en el directorio raíz
		stbuf->st_mode = S_IFREG | 0444; // fichero con permisos de lectura
		stbuf->st_nlink = 1;
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;
		
		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size/512 + (stbuf->st_size%512)? 1 : 0; // bloques ocupados por el fichero
	} else if (strcmp(path, "/BIG") == 0) {
		stbuf->st_mode = S_IFDIR | 0755; // directorio con permisos de lectura y ejecución
		stbuf->st_nlink = 2; // directorio con dos enlaces
		stbuf->st_uid = mis_datos->st_uid; // usuario propietario
		stbuf->st_gid = mis_datos->st_gid; // grupo propietario
		
		stbuf->st_atime = mis_datos->st_atime; // último acceso
		stbuf->st_mtime = mis_datos->st_mtime; // última modificación
		stbuf->st_ctime = mis_datos->st_ctime; // última modificación de los metadatos
		stbuf->st_size = 1024; // tamaño del directorio
		stbuf->st_blocks = 2; // bloques ocupados por el directorio
	} else if (strcmp(path, "/little") == 0) {
		stbuf->st_mode = S_IFDIR | 0755; // directorio con permisos de lectura y ejecución
		stbuf->st_nlink = 2; // directorio con dos enlaces
		stbuf->st_uid = mis_datos->st_uid; // usuario propietario
		stbuf->st_gid = mis_datos->st_gid; // grupo propietario
		
		stbuf->st_atime = mis_datos->st_atime; // último acceso
		stbuf->st_mtime = mis_datos->st_mtime; // última modificación
		stbuf->st_ctime = mis_datos->st_ctime; // última modificación de los metadatos
		stbuf->st_size = 1024; // tamaño del directorio
		stbuf->st_blocks = 2; // bloques ocupados por el directorio
	} else if ((strstr(path, "BIG") != NULL) && (i= buscar_fichero(path+4, mis_datos)) >= 0) { // fichero en el directorio /BIG
		stbuf->st_mode = S_IFREG | 0444; // fichero con permisos de lectura
		stbuf->st_nlink = 1;
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;

		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size/512 + (stbuf->st_size%512)? 1 : 0; // bloques ocupados por el fichero
	} else if ((strstr(path, "little") != NULL) && (i = buscar_fichero(path+7, mis_datos)) >= 0) { // fichero en el directorio /little
		stbuf->st_mode = S_IFREG | 0444; // fichero con permisos de lectura
		stbuf->st_nlink = 1;
		
		stbuf->st_uid = mis_datos->st_uid;
		stbuf->st_gid = mis_datos->st_gid;
		
		stbuf->st_atime = mis_datos->st_atime;
		stbuf->st_mtime = mis_datos->st_mtime;
		stbuf->st_ctime = mis_datos->st_ctime;

		stbuf->st_size = strlen(mis_datos->contenido_ficheros[i]);
		stbuf->st_blocks = stbuf->st_size/512 + (stbuf->st_size%512)? 1 : 0; // bloques ocupados por el fichero
	} else
		res = -ENOENT;
	
	return res;
}

/***********************************
 * */

static int mi_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
		
	(void) offset;
	(void) fi;

	if (*path !=  '/') // Si no es una dirección se reporta error
		return -ENOENT;
	
	if (strcmp(path, "/") == 0) // Si es la raíz
	{
		if(filler(buf, "." , NULL, 0)!=0) return -ENOMEM; // añadir "." al directorio
		if(filler(buf, "..", NULL, 0)!=0) return -ENOMEM; // añadir ".." al directorio
		if (filler(buf,"BIG", NULL, 0) != 0) return -ENOMEM; // añadir "BIG" al directorio	
		if (filler(buf,"little", NULL, 0) != 0) return -ENOMEM; // añadir "little" al directorio

		for (int i=0; i< mis_datos->numero_ficheros; i++)
		{
			if (filler(buf,mis_datos->nombre_ficheros[i], NULL, 0) != 0)
				return -ENOMEM;
		}
	}
	else if	(strstr(path, "BIG") != NULL)
	{
		if(filler(buf, "." , NULL, 0)!=0) return -ENOMEM; // añadir "." al directorio
		if(filler(buf, "..", NULL, 0)!=0) return -ENOMEM; // añadir ".." al directorio
		for(int i = 0; i < mis_datos->numero_ficheros; i++)
		{
			if (strlen(mis_datos->contenido_ficheros[i]) > 255) // Si el nombre del fichero es mayor a 255 caracteres
			{
				if (filler(buf,mis_datos->nombre_ficheros[i], NULL, 0) != 0)
					return -ENOMEM;
			}
		}
	}
	else if	(strstr(path, "little") != NULL)
	{
		if(filler(buf, "." , NULL, 0)!=0) return -ENOMEM; // añadir "." al directorio
		if(filler(buf, "..", NULL, 0)!=0) return -ENOMEM; // añadir ".." al directorio
		for(int i = 0; i < mis_datos->numero_ficheros; i++)
		{
			if (strlen(mis_datos->contenido_ficheros[i]) < 256) // Si el nombre del fichero es menor a 256 caracteres
			{
				if (filler(buf,mis_datos->nombre_ficheros[i], NULL, 0) != 0)
					return -ENOMEM;
			}
		}
	}

	return 0;

}

/***********************************
 * */
static int mi_open(const char *path, struct fuse_file_info *fi)
{
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;

	int	file_descriptor;
	int	chapter_position;
	
	if ((fi->flags & 3) != O_RDONLY) return -EACCES; // Se intentó abrir en modo escritura
	if (strstr(path, "BIG") != NULL)
	{
		chapter_position = buscar_fichero(path+4, mis_datos); // buscar capítulo /BIG/...
		if (chapter_position < 0) return -ENOENT; // no existe el fichero
	}
	else if (strstr(path, "little") != NULL)
	{
		chapter_position = buscar_fichero(path+7, mis_datos); // buscar capítulo /little/...
		if (chapter_position < 0) return -ENOENT; // no existe el fichero
	}
	else
	{
		chapter_position = buscar_fichero(path, mis_datos); // buscar capítulo /...
		if (chapter_position < 0) return -ENOENT; // no existe el fichero
	}
	
	fi->fh = chapter_position; // voy a utilizar el descriptor para guardar la posición del fichero en el array de ficheros
	return (0);
}


/***********************************
 * */
static int mi_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	struct structura_mis_datos *mis_datos= (struct structura_mis_datos *) fuse_get_context()->private_data;
	char *contenido_ficheros = mis_datos->contenido_ficheros[fi->fh];

	size_t length = strlen(contenido_ficheros);
	if (offset < length) {
		if (offset + size > length) size = length - offset;
		memcpy(buf, contenido_ficheros + offset, size);
		return (size);
	} 
	else 
		return (0);

}


/***********************************
 * operaciones FUSE
 * */
static struct fuse_operations basic_oper = {
	.getattr	= mi_getattr,
	.readdir	= mi_readdir,
	.open		= mi_open,
	.read		= mi_read,
};


/***********************************
 * */
int main(int argc, char *argv[])
{
	struct structura_mis_datos *mis_datos;
	
	mis_datos=malloc(sizeof(struct structura_mis_datos));
	
	// análisis parámetros de entrada
	if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-')) error_parametros();

	mis_datos->fichero_inicial = strdup(argv[argc-2]); // fichero donde están los capítulos
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    leer_fichero(mis_datos);
    
/*    int i;
    for(i=0; i<mis_datos->numero_ficheros; i++)
    {
		printf("----->  %s\n", mis_datos->nombre_ficheros[i]);
		printf("%s",mis_datos->contenido_ficheros[i]);
	}
	exit(0);
*/
	
	return fuse_main(argc, argv, &basic_oper, mis_datos);
}
