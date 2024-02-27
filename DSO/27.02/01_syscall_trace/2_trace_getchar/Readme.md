
--
#Llamada _read_ al sistema operativo

Compilar getchar0.c:
	gcc getchar0.c -o getchar0

Ejecutar y comprobar la salida:
	./getchar0	# pulsar un tecla seguido de enter

	./getchar0	# enviar señal SIGUSR1 desde otro CLI

Podemos concluir que el estado de espera en el que entra el proceso cuando
invoca getchar() es un bloqueado interrumpible puesto que ejecuta el manejador
aun cuando no se ha producido el evento en el que estaba esperando (recibir un
caracter por teclado).
Aun siendo un estado de bloqueado interrumpible, el programa getchar0 sigue en
espera de pulsaciones de teclado. Esto es debido a que la llamada signal() por
defecto pide al SO que si el proceso está bloqueado en una syscall, que intente
rearrancarla (SA_RESTART).

Volver a repetir los experimentos pero ejecutando getchar0 con strace:
	strace ./getchar0	# se muestran todas las llamadas la sistema operativo

Podemos observar que la funcion getchar() es un envoltorio (wrapper), al igual
que fgetc(), fgets() y getc() de la syscall read().

--
# Modificar el comportamiento del handler de señal para no rearrancar la syscall

Compilar getchar1.c:
	gcc getchar1.c -o getchar1

Ejecutar y comprobar la salida:
	./getchar1	# pulsar un tecla seguido de enter

	./getchar1	# enviar señal SIGUSR1 desde otro CLI

Podemos observar que getchar() retorna EOF tras la ejecución del manejador de
la interrupción. Esto ha sido forzado en la llamada a sigaction() al poner a cero
el campo sa_flags de la estructura sigaction que se la pasa como argumento.
Recordad que por defecto, la funcion signal() invoca sigaction() con el flag de
SA_RESTART.

