Máximo García Aroca / Rodrigo Hernández Barba
Se han entregado 4 archivos:

1. berryclip.c: el driver para la berryclip con las operaciones read y write para los LEDS, write para el SPEAKER y read para BUTTONS. Cada uno sigue las especificaciones exigidas en el guión de la práctica. 

2. Makefile: el comando para cargar el driver es "make MODULE="nombre" MILLISECONDS="milisegundos para gap de buttons"".

3. bin2char: es un pequeño script en bash que sirve para introducir la configuración de leds en el /dev/leds poniendo un número binario como entrada al script. Ejemplo: "./bin2char 00101010 > /dev/leds" Convierte 00101010 en un asterisco '*' que en ASCII corresponde con un 42. La función leds_write interpreta el caracter y enciende los leds correspondientes.

4. dev2bin: hace la función inversa a bin2char, se le proporciona un caracter leído de /dev/leds y lo convierte a número binario. Ejemplo "./dev2bin /dev/leds" o "cat /dev/leds | xargs ./dev2bin".
