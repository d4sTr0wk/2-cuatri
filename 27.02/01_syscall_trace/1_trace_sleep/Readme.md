
--
#Llamada _sleep_ al sistema operativo

Compilar sleep0.c:
	gcc sleep0.c -o sleep0

Ejecutar y comprobar la salida:
	./sleep0	# dejar pasar 20 segundos a que termine
	echo $?		# debe mostrar 0

	./sleep0	# ^C tras un par de segundos -> enviar señal SIGINT
	echo $?		# debe mostrar 130 -> proceso terminado por señal
			# para mas detalles: man 3 sleep
Podemos concluir que el estado de espera en el que entra el proceso cuando
invoca sleep() es un bloqueado interrumpible puesto que ejecuta el manejador
aun cuando no se ha producido el evento en el que estaba esperando (el paso de
20 segundos). El manejador por defecto de SIGINT es terminar el proceso.

Volver a repetir los dos experimentos (esperando e interrumpiendo con ^C),
pero ejecutando sleep0 con strace:
	strace ./sleep0	# se muestran todas las llamadas la sistema operativo
			# que realiza sleep0 y sus valores de retorno
Podemos observar que la funcion sleep() es un envoltorio (wrapper), al igual
que usleep() o nanosleep(), de clock_nanosleep()

--
# Incluir un manejador de señales para SIGINT y para SIGUSR1

Compilar sleep1.c:
	gcc sleep1.c -o sleep1

Ejecutar y comprobar la salida:
	./sleep1	# ^C tras un par de segundos
	echo $?		# debe mostrar 18 (numero de segundos restante)
Al tener un manejador se señal SIGINT que no termina el proceso, la llamada al
sistema sleep(), que fue interrumpida, retorna con el número de segundos que
le quedaban por esperar. La llamada sleep() no se reinicia si es interrumpida
por una señal.

Usando strace, volver a repetir los dos experimentos: uno esperando y otro
enviando una señal SIGUSR1 desde otro CLI (interfaz de linea de comandos):
	ps -ux			# para ver el PID del proceso
	kill -SIGUSR1 <PID>
Ahora vemos el resto de syscalls del programa (ejecución del manejador)

Podemos observar que la funcion signal() es un wrapper de rt_sigaction() que es
la llamada al sistema que permite definir el comportamiento de las señales con
mucho más detalle a través de la estructura sigaction.
Aunque el flag SA_RESTART está indicado en el establecimiento de los
manejadores de las señales SIGINT y SIGUSR1, sleep() no rearranca sino que
termina retornando el número de segundos que le quedaban por esperar.

Es responsabilidad del programador comprobar los valores de retorno de las
llamadas a funciones para realizar el tratamiento correcto que requiera su
aplicación. En el caso de la llamada a sleep(), si no retorna cero, debe
invocar nuevamente a sleep() con el tiempo restante.


