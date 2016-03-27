CI-3825 Sistemas De Operacion I
Proyecto I -Chat 
Midaysa Palacios y Francisco Sucre

Nuestro proyect "UsoDisco" consta de un solo programa 

- El programa UsoDisco recorre un directorio base y calcula la cantidad de bloques
de 512 bytes ocupa el directorio, incluyendo la suma de los tamanos de sus subdirectorios,
puede trabajar con multiples hilos.


Instrucciones:

	Compilacion:
	
		Para crear el ejecutable de nuestro programa solo se requiere
		de un commando, ubicandonos en el directorio base de nuestro
		proyecto ejecutamos el siguiente comando:

		"make proyect" o "make"

		Esto creara el ejecutable UsoDisco. Si por alguna razon
		necesita borrar dichos ejectubles, puede hacerlo con el siguiente
		comando en la linea de comandos.

		"make clean"

	Ejecucion:

		Abrimos nuestras lineas de comando

		UsoDisco: Para ejecutar el servidor basta con correr el siguiente comando
	
			"./UsoDisco"

		El programa cuenta con 4 atributos adicionales que se le pueden agregar en la linea
		de commandos

			-n <nivel de recurrencia>: indica cuantos hilos utilizara el programa, en caso
			de no incluirlo, el nivel por defecto es 1
				
				./UsoDisco -n <nivel de recurrencia>
			
			-d <directorio>: el directorio base a recorrer, en caso
			de no incluirlo, el directorio por defecto es el actual
			
			-o <nombre de archivo>: Archivo en el que se guardara todos los directorios
			y su tamano en bloques

			-h <nombre de archivo>: Opcion para imprimir el instructivo del programa, no se puede
			escoger otra opcion junto con esta

		Nota: El programa puede recibir los primeros 3 argumentos sin repetirse y en cualquier orden,
		si se repiten los argumentos el programa fallara y dara un mensaje de error
			

Archivos:

	UsoDisco: Archivo fuente del proyecto

	messages.c: codigo fuente que contiene los mensajes utilizados en el proyecto, tambien
	contiene algunas funciones de uso comun

	messages.h: archivo header de commons.c utilizado para importar sus constantes y funciones al programa

	ConectarseAlLdc.sh: script utilizado para conectarse a una computadora del LDC utilizando la cuenta 10-10717

	readme: Archivo explicativo que informa el uso del directorio, se utilizo para poder inicializar
	dicho directorio en el repositorio Git utilizado en el proyecto

	Makefile: Archivo que automatiza la compilacion de los programas del proyecto completo

	limpiarYEnviarAlLdc.sh: script utilizado para enviar los archivos del proyecto a una direccion fija del LDC con
	la cuenta 10-10717

