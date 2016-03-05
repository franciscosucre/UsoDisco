/*
 * 	messages.c
 *
 *	Archivo en el cual se encuentran los mensajes de error y funciones utilizadas
 *	para su manejo
 *
 *  Created on: Jan 20, 2016
 *      Author: francisco y midaysa
 */

#include <stdlib.h>
#include <string.h>             // strlen
#include <stdio.h>              // printf
#include <time.h>               // sleep
#include <fcntl.h>              // O_NONBLOCK, etc
#include <string.h>             // strlen
#include <sys/stat.h>           // mkfifo

#define BASIC_PERMISSIONS 0666
#define MSG_LEN 500
#define MENU_LEN 800
#define ERROR_LEN 100

// Mensajes de error

const char pthreadCreateError[ERROR_LEN] = "pthreadCreateError";
const char pthreadJoinError[ERROR_LEN] = "pthreadJoinError";
const char mkfifoError[ERROR_LEN] = "mkfifo Error";
const char selectError[ERROR_LEN] = "select Error";
const char openDirError[ERROR_LEN] = "openDir Error";
const char openError[ERROR_LEN] = "open Error";
const char getcwdError[ERROR_LEN] = "getcwd() Error";
const char mallocError[ERROR_LEN] = "malloc Error";
const char dirNotExistsError[ERROR_LEN] = "El Directorio deseado no existe";
const char argNumError[ERROR_LEN] = "Numero Incorrecto de Argumentos";
const char argOrdError[ERROR_LEN] = "Orden Incorrecto de Argumentos";
const char invOrdError[ERROR_LEN] = "Argumentos Invalidos";


// Mensajes de sistema

const char helpMenu[MENU_LEN] = "\n"
		"Sintaxis\n"
		"\n"
		"UsoDisco [-h] | [-n i] [-d directorio] [-o salida ]\n"
		"\n"
		"-h: muestra por pantalla un mensaje de ayuda (sintaxis, descripción de parámetros, etc.) "
		"y termina\n"
		"\n"
		"-n i: nivel de concurrencia solicitado. Por defecto crea un solo hilo trabajador\n"
		"-d directorio: especifica un directorio desde donde calcula el espacio utilizado. "
		"Por defecto hace el cálculo desde el directorio actual\n"
		"-o salida: archivo que contendrá la salida con la lista de directorios y el espacio en "
		"bloques ocupado por los archivos regulares. El valor por defecto es la salida estándar\n"
		"\n";

// Ordenes Cliente->Servidor

/*
 * Function:  errorAndExit
 * --------------------
 *  Imprime un error y sale del programa.
 *
 *	errorMessage: el mensaje de error a utilizar
 *
 *  returns: void
 */
void errorAndExit(const char* errorMessage)

{
	perror(errorMessage);
	exit(0);
}



