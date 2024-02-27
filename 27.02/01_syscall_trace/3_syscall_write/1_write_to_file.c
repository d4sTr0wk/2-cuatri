// Diferencias en el buffering entre las llamadas al sistema y las llamadas a
//  funciones de librería
// Usar strace para comprobar las llamadas que se hacen realmente

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
   char datos[5] = "Hola ";
   char *filename = "prueba.out";
   if (argc > 1) {
      filename = argv[1];
   }

   FILE *f = fopen(filename, "w+"); // open for read & write; truncate
   int i = setvbuf(f, malloc(10000), _IOFBF, 10000); // just after fopen
   if (i != 0) {
      perror("setvbuf");
   }

   // escribimos en un fichero 1000 veces 5 chars, en total 5000 bytes
   for (i = 0; i < 1000; i++) {
      fwrite(datos, sizeof(char), 5, f);
   }

   // vamos al principio
   fseek(f, 0, 0);

   // leemos de este fichero 3 veces 5 chars, en total 15 bytes
   fread(datos, sizeof(char), 5, f);
   fread(datos, sizeof(char), 5, f);
   fread(datos, sizeof(char), 5, f);

   fclose(f);

   // leemos otra vez pero con la llamada al sistema en vez de usar la libreria de c
   int fd = open("prueba.out", O_RDONLY);

   read(fd, datos, 5);
   read(fd, datos, 5);
   read(fd, datos, 5);

   close(fd);
   exit(EXIT_SUCCESS);
}
