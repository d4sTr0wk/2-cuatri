// Accedemos al fichero mapeandolo en memoria
// Comprobar con strace las llamadas realizadas

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


int main(int argc, char *argv[])
{
   char *filename = "prueba.out";
   char *mystring = "Hello";
   switch (argc) {
      case 3: mystring = argv[2];
      case 2: filename = argv[1];
   }
   int datasize = strlen(mystring);

   // abrimos el fichero
   int fd = open(filename, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR);
   if (fd == -1) {
      perror("open");
      exit(EXIT_FAILURE);
   }
   struct stat statbuf;
   if (fstat(fd, &statbuf) == -1) { // get information about a file
      perror("fstat");
      exit(EXIT_FAILURE);
   }

   long filesize = statbuf.st_size;
   long pagesize = sysconf(_SC_PAGESIZE);
   if (filesize < pagesize) {
      ftruncate(fd, pagesize);   // extend file so we can map a page
   }

   // mapeamos la primera pagina del fichero en memoria
   // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
   char *datos = mmap(NULL, pagesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd , 0);
   if (datos == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
   }

   // si accedemos al espacio fuera del tamaño real del fichero -> SEG FAULT
   if (datasize > pagesize) {
      datasize = pagesize;
   }
   // leemos los primeros caracteres del fichero a traves del mapeo y los pintamos en pantalla
   for (int i = 0; i < datasize; i++) {
      write(1, datos+i, 1);   // ssize_t write(int fd, const void *buf, size_t count);
      //fwrite(&datos[i], 1, 1, stdout); // size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
   }

   // escribimos en memoria -> fichero
   sprintf(datos, "%s", mystring);
   // strcpy(datos, mystring);
   // for (int i = 0; i < datasize; i++) datos[i] = mystring[i];

   if (datasize > filesize) {
      filesize = datasize; // update size of file
   }

   // leemos del fichero: ssize_t read(int fd, void *buf, size_t count);
   char buf[pagesize];
   read(fd, buf, datasize);

   // pintamos la lectura del fichero
   printf("\n%s\n", buf);

   // libreamos el mapeo de memoria y cerramos el fichero
   munmap(datos, pagesize);
   if (filesize < pagesize) {
      ftruncate(fd, filesize);   // crop file to the original size
   }
   close(fd);

   exit(EXIT_SUCCESS);
}
