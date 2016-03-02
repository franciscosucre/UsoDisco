/*
 * commons.c
 *
 *	Contiene los mensajes que se utilizan en el programa y ademas funciones comunes para el cliente
 *	y el servidor
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

// Mensajes de error

const char *mkfifoError = "mkfifo Error";
const char *selectError = "select Error";
const char *openError = "open Error";
const char *getcwdError = "getcwd() Error";
const char *mallocError = "malloc Error";
const char *dirNotExistsError = "El Directorio deseado no existe";
const char *argNumError = "Numero Incorrecto de Argumentos";
const char *argOrdError = "Orden Incorrecto de Argumentos";
const char *invOrdError = "Argumentos Invalidos";
const char *termSizeError = "El terminal es muy pequeno para correr este programa.\n";
const char *rvError = "rvError ";
const char *writeToMySelf = "No puedes escribir mensajes para ti mismo";


// Mensajes de sistema

const char *LogOutMessage = "Cerrando Sesion... Thank You For Using Our Chat Services!";
const char *LogOutServerMessage = " ha cerrado sesion!";
const char *noUserSelectedMessage = "No le esta escribiendo a ningun usuario!\n";
const char *defaultStatus = "Sin Status";
const char *defaultServer = "/tmp/servidor";
const char *defaultUsername = "System";
const char *loginResultMessage = "Resultado Del Inicio De Sesion:";
const char *helpMenu = "\n"
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

const char *ordenQuien = "-quien";
const char *ordenEscribir = "-escribir";
const char *ordenEstoy = "-estoy";
const char *ordenSalir = "-salir";
const char *ordenAyuda = "-ayuda";
const char *ordenCambiarConversacion = "-cambiarConversacion";
const char *ordenInvalida = "Orden Invalida";

const char *serverStartMessage = "Iniciando servidor!\n";
const char *welcomeMessage = "--------- Mega Servicio De Chat! Bienvenido! ---------\n \n";
const char *successMessage = "Operacion Exitosa";
const char *userNotFoundMessage = "El usuario al que quiere escribirle no se encuentra conectado";
const char *userNameNotAvaible = "El nombre de usuario no se encuentra disponible, por favor escoja otro";

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

char* getErrorMessage(const char* errorMessage,int line, char* file)

{
	// Convertimos el numero de linea en un String

	char* lineString;
	lineString = (char *) malloc(sizeof(line));
	if (lineString == NULL)
	{
		errorAndExit(mallocError);
	}
	sprintf(lineString, "%d", line);

	// Inicializamos la variable en la que devolveremos el error

	char* finalMessage;
	finalMessage = (char *) malloc(strlen(errorMessage) + strlen(" at line ")
			+ strlen(lineString) + strlen(" in file ") + strlen(file));
	if (finalMessage == NULL)
	{
		errorAndExit(mallocError);
	}

	// Iniciamos el mensaje con el error

	finalMessage = strcpy(finalMessage, errorMessage);

	// Agregamos la linea en la que ocurrio

	strcat(finalMessage, " at line ");
	strcat(finalMessage, lineString);

	// Agregamos el archivo donde ocurrio

	strcat(finalMessage, " in file ");
	strcat(finalMessage, file);

	// Retornamos el mensaje final

	return finalMessage;

}



// Separa un string con el delimitador seleccionado y devuelve la palabra seleccionada con index
char* getWord(char* string,char* delimeter,int index)

{
	// Si la lista es nula entonces abandonamos la funcion

	if (string == NULL)
	{
		return NULL;
	}

	char* stringCopy; //Copia del string original para no cambiarlo
	char* word; // Palabra a ser obtenida
	int i; // Iterador
	char** stringCopyPointer; // Apuntador a la copia del string necesaria para la funcion strsep
	char* stringCopyAddress;
	char* stringCopyPointerAddress;

	// Reservamos la memoria para la copia del string

	stringCopy = (char *) malloc(strlen(string));
	stringCopyAddress = stringCopy;
	strcpy(stringCopy, string);
	stringCopyPointer  = &stringCopy;
	word = (char *) malloc(strlen(string));

	// Recorremos la lista de palabras obteniendo sus palabras una a una

	for ( i = 0; i <= index; i = i + 1 )
	{
		strcpy(word, strsep(stringCopyPointer,delimeter));

		// Si la palabra es igual al string completo, nos salimos de la funcion

		if(stringCopy == NULL)
		{
			return word;
		}

	}

	// Si no ocurrieron ninguno de los casos anteriores entonces debimos obtener la palabra deseada

	free(stringCopyAddress);
	return word;
}


/*
 * Function:  writeFull
 * --------------------
 *  Escribe lo que le sobra a token dentro de dst
 *
 *  token: String original
 *
 *  dst: String destino
 *
 *  returns: void
 */
void writeFull(char *token, char dst[])
{
    char tmp[MSG_LEN] = "";

    while (token != NULL)
    {
        strcat(tmp, token);
        strcat(tmp, " ");
        token = strtok(NULL, " ");
    }

    strcpy(dst, tmp);
    dst[strlen(dst)-1] = 0;
}
