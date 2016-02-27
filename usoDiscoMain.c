/*
 * usoDiscoMain.c
 *
 *  Created on: Feb 25, 2016
 *      Author: francisco
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include <unistd.h>				// getcwd
#include <string.h>             // strlen
#include <stdio.h>              // printf
#include <stdlib.h>				// exit
#include <pthread.h>			// Hilos
#include <dirent.h>				// dirent
#include <sys/types.h>
#include <sys/stat.h>

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "None"

struct thread_data
{
   int  thread_id;
   int  size;
   char directory[MSG_LEN];
};

int main(int argc, char *argv[])
{
	char directorioInicial[MSG_LEN];		// Directorio a analizar
	char archivoSalidaNombre[MSG_LEN];		// Nombre del Archivo de salida
	FILE * archivoSalida;					// Archivo de salida
	int nivelConcurrencia;					// Numero de hilos a crear
	int i;									// Iterador
	char* cwd;								// Apuntador utilizado para obtener el directorio actual
	int salidaSwith,levelSwith,				// Variables booleanas que indican si ya se dio un argumento
	dirSwith;
	int rc;									//
	void *status;							//
	int numDirectorios;						// Numero actual de subdirectorios
	char listaDirectorios[MAXDIR][MSG_LEN]; // Lista de directorios a analizar
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios

	salidaSwith = 0;
	levelSwith = 0;
	dirSwith = 0;

	/* Se asignan los valores por defectos */
	nivelConcurrencia = DEFAULT_CONCURRENCY;
	strcpy(archivoSalidaNombre,DEFAULT_FILE);
	archivoSalida = NULL;

	/* Obtenemos el directorio Actual */
	cwd = getcwd(directorioInicial, sizeof(directorioInicial));
	if ((cwd) != NULL)
	{
		fprintf(stdout, "Directorio Actual: %s\n", directorioInicial);
	}
	else
	{
		errorAndExit(getErrorMessage(getcwdError,__LINE__, __FILE__));
	}

	/* Caso 1: Se recibio 1 solo argumento */
	if (argc == 2)
	{
		/* Caso 1.1: Se recibio el argumento correcto */
		if ((strcmp(argv[1],"-h")) == 0)
		{
			printf("%s", helpMenu);
			exit(0);
		}
		/* Caso 1.2: Se recibio el argumento incorrecto */
		else
		{
			printf("%s", invOrdError);
			exit(0);
		}
	}

	/* Caso 2: Se recibieron varios argumentos */
	else if (argc == 3 || argc == 5 || argc == 7)
	{
		/* Recorremos la lista de argumentos */
        for (i=1; i<argc; i = i + 2)
        {
        	/* Caso 2.1: Se recibio el argumento del numero de concurrencia */
        	if ((strcmp(argv[i],"-n")) == 0 && levelSwith == 0)
			{
        		nivelConcurrencia = atoi(argv[i + 1]);
        		levelSwith = 1;
			}
        	/* Caso 2.2: Se recibio el argumento del numero de concurrencia */
        	else if ((strcmp(argv[i],"-d")) == 0 && dirSwith == 0)
			{
        		strcpy(directorioInicial,argv[i + 1]);
        		dirSwith = 1;
			}
        	/* Caso 2.3: Se recibio el argumento del numero de concurrencia */
        	else if ((strcmp(argv[i],"-o")) == 0 && salidaSwith == 0)
			{
        		strcpy(archivoSalidaNombre,argv[i + 1]);
        		archivoSalida = fopen (archivoSalidaNombre, "w+");
        		salidaSwith = 1;
			}
        	/* Caso 2.4: Se recibieron argumentos en un formato invalido */
    		else
    		{
    			printf("%s", invOrdError);
    			exit(0);
    		}
        }

	}

	/* Caso 3: Se recibieron mas argumentos de los posibles */
	else if (argc > 7)
	{
		printf("%s", argNumError);
		exit(0);
	}

	/*
	pthread_t hilos[nivelConcurrencia];
	struct thread_data thread_data_array[nivelConcurrencia];
*/


	/* Creacion De Hilos */

	/* A esto hay que
	 *
	for(i=0; i<nivelConcurrencia; i++)
	{
	   thread_data_array[i].thread_id = i;
	   thread_data_array[i].directory = "";
	   rc = pthread_create(&hilos[i], NULL, funcHilo, (void *) &thread_data_array[i]);
	}
	*/



	directorioTemp = opendir(directorioInicial);
	if (directorioTemp == NULL)
	{
		perror("opendir");
		exit(-1);
	}

	/* Recorremos los archivos del directorio original */
	while ((dp = readdir(directorioTemp)) != NULL)
	{
		stat(dp->d_name, &bufferDeArchivo);

		/* Caso 1: El archivo es un directorio */
		if (S_ISDIR(bufferDeArchivo.st_mode))
		{
			printToOutput(archivoSalida,"directorio: %s\n",dp->d_name);
		}
		/* Caso 2: El archivo es un fichero */
		else
		{
			printToOutput(archivoSalida,"directorio: %s\n",dp->d_name);
		}
	}


	/* Retorno De Hilos */
	/*
	for(i=0; i<nivelConcurrencia; i++)
	{
	   rc = pthread_join(hilos[i], &status);
	   if (rc)
	   {
		  printf("ERROR; return code from pthread_join() is %d\n", rc);
		  exit(­1);
	   }
	printf("Main: completed join with thread %ld having a status of %ld\n",i,(long)status);
	}
	*/

	printf("\n");
	printf("nivelConcurrencia = %d\n",nivelConcurrencia);
	printf("directorio = %s\n",directorioInicial);
	printf("archivoSalidaNombre = %s\n",archivoSalidaNombre);

	if (archivoSalida != NULL)
	{
		fclose(archivoSalida);
	}
	close(directorioTemp);
	return 0;
}

/*
 * Function:  funcHilo
 * --------------------
 *  Funcion que ejecturan los hilos
 *
 *  threadarg: Informacion necesaria para que el hilo creado pueda completar la funcion
 *
 *  returns: void
 */
void *funcHilo(void *threadarg)
{
	printf("\n");
	return 0;
}

/*
 * Function:  printToOutput
 * --------------------
 *  Funcion que imprime UN string con un patron a la salida seleccionada
 *
 *  output: Apuntador al archivo seleccionado como salida del programa principal, este
 *  puede venir en NULL lo cual significa que la persona no especifico ningun archivo
 *  de salida
 *
 *  pattern: patron con el cual se escribira string
 *
 *  messageToPrint: String a imprimir
 *
 *  returns: void
 */
void printToOutput(FILE * output,char pattern[MSG_LEN],char messageToPrint[MSG_LEN])
{
	/* Caso 1: No hay archivo de salida seleccionado */
	if (output == NULL)
	{
		printf(pattern,messageToPrint);
	}
	/* Caso 2: Si hay archivo de salida seleccionado*/
	else
	{
		fprintf(output,pattern,messageToPrint);
	}
}
