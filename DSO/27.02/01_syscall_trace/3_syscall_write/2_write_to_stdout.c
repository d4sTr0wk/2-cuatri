// Diferentes formas de llamar al sistema desde un programa en C
// Comprobar con strace las llamadas que se realizan

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>        // For SYS_xxx definitions

int main()
{
    int n1, n2, n3, n4;
    // Escribe por pantalla usando la libreria de C stdio.h
    n1 = fprintf(stdout, "Salida printf() desde C\n");

    // Escribe por pantalla usando el envoltorio de la llamada al sistema en unistd.h 
    n2 = write(STDOUT_FILENO, "Salida write() desde C\n", 23);   // STDOUT_FILENO = 1

    // Escribe usando la funcion de C en unistd.h que llama al sistema de forma generica
    n3 = syscall(SYS_write, STDOUT_FILENO, "Salida syscall() desde C\n", 25);  // SYS_write = 4

    #if defined( __x86_64__ ) || defined( __i386__ )  // Intel x86 & x64
    // Usa ensamblador para hacer la llamada al sistema con la interrupcion 80 hex
    char *mensaje = "Salida write con syscall desde asm\n";  // longitud 35 caracteres
    asm("movq $1, %%rax \n\t"   // use the `write` [fast] syscall
        "movq $1, %%rdi \n\t"   // write to stdout
        "movq %1, %%rsi \n\t"   // use string in var mensaje
        "movq $35, %%rdx \n\t"  // write 35 characters
        "syscall \n\t"          // make syscall
        "movl %%eax, %0"
        : "=r" (n4)             // salida en n4
        : "r" (mensaje)         // variables de entrada, la 'r' indica registro
        : "%rax", "%rdi", "%rsi", "%rdx"   // registros usados para que el compilador lo sepa
       );
    #elif defined( __arm__ )    // ARM
    // Usa ensamblador para hacer la llamada al sistema con la instruccion  swi/svc
    char *mensaje = "Salida write con SWI/SVC desde asm\n";  // longitud 35 caracteres
    asm( "mov r7, #4  \n\t"
         "mov r0, #1  \n\t"
         "mov r1, %1  \n\t"
         "mov r2, #35 \n\t"
         "svc 0 \n\t"
         "mov %0, r0"
             : "=r" (n4)        // salida en n4
             : "r" (mensaje)    // variables de entrada, la 'r' indica registro
             : "r0", "r1", "r2", "r7" // clobberer registers
       );
    #else
    printf("Arquitectura ISA no soportada\n");
    #endif

    printf("Se han impreso %d, %d, %d y %d caracteres respectivamente\n", n1, n2, n3, n4);

    return 0;
}
