/*
 * usoDiscoMain.c
 *
 *  Created on: Feb 25, 2016
 *      Author: francisco
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include "directoryStack.h"     // Pila de directorios
#include "pthreadQueue.h"       // Cola de hilos
#include <unistd.h>				// getcwd
#include <string.h>             // strlen
#include <stdio.h>              // printf
#include <stdlib.h>				// exit
#include <pthread.h>			// Hilos
#include <dirent.h>				// dirent
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "stdout"

struct thread_data
{
   int  thread_id;
   int  size;
   char directory[MSG_LEN];
};

FILE * archivoSalida;					// Archivo de salida
int numDirectorios;						// Numero de directorios en la lista
int dirAnalizar;						// Indice del directorio a analizar
char listaDirectorios[MAXDIR][MSG_LEN]; // Lista de directorios a analizar
pthread_mutex_t numTotalBlocksLock;
int numTotalBlocks;
struct directoryStack pilaDirectorios;
struct pThreadQueue colaHilos;

void *funcHilo(void *threadarg);

int main(int argc, char *argv[])
{
	char directorioInicial[MSG_LEN];		// Directorio a analizar
	char archivoSalidaNombre[MSG_LEN];		// Nombre del Archivo de salida
	int nivelConcurrencia;					// Numero de hilos a crear
	int i;									// Iterador
	char* cwd;								// Apuntador utilizado para obtener el directorio actual
	int salidaSwith,levelSwith,				// Variables booleanas que indican si ya se dio un argumento
	dirSwith;
	int rc;									//
	void *status;							//
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	int bloquesTotal;						// Numero de bloques en total
	struct thread_data threads[MAXDIR];		//
	pthread_t hilos[MAXDIR];


	salidaSwith = 0;
	levelSwith = 0;
	dirSwith = 0;

	/* Se asignan los valores por defectos */
	nivelConcurrencia = DEFAULT_CONCURRENCY;
	strcpy(archivoSalidaNombre,DEFAULT_FILE);
	archivoSalida = stdout;
	strcpy(directorioInicial,".");

	/* Caso 1: Se recibio 1 solo argumento */
	if (argc == 2)
	{
		/* Caso 1.1: Se recibio el argumento correcto */
		if ((strcmp(argv[1],"-h")) == 0)
		{
			printf("%s\n", helpMenu);
			exit(0);
		}
		/* Caso 1.2: Se recibio el argumento incorrecto */
		else
		{
			printf("%s\n", invOrdError);
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
        		printf("El directorio inicial es %s",directorioInicial);
				directorioTemp = opendir(directorioInicial);

				if (directorioTemp == NULL)
				{
					printf("%s\n",dirNotExistsError);
					exit(-1);
				}
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
    			printf("%s\n", invOrdError);
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

	bloquesTotal = 0;
	dirAnalizar = 0;

	printf("Llegue hasta aqui\n");

	struct thread_data thread_data_array[nivelConcurrencia];
	struct thread_data thread_data;

	initializeStack(&pilaDirectorios);
	initializeQueue(&colaHilos);

	pushToStack(&pilaDirectorios,directorioInicial);

	printf("El directorio es %s\n",pilaDirectorios.listaDirectorios[0]);

	printf("stackSize(&pilaDirectorios) es %d\n",stackSize(&pilaDirectorios));

	while (stackSize(&pilaDirectorios) > 0)
	{
		popFromStack(&pilaDirectorios,thread_data.directory);
		printf("thread_data.directory = %s\n",thread_data.directory);
		rc = pthread_create(&hilos[0], NULL, funcHilo, (void *) &thread_data);
		rc = pthread_join(hilos[0], &status);
	}


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
	printf("numTotalBlocks = %d\n",numTotalBlocks);


	if (archivoSalida != NULL)
	{
		fclose(archivoSalida);
	}
	closedir(directorioTemp);
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
//	struct thread_data
//	{
//	   int  thread_id;
//	   int  size;
//	   char directory[MSG_LEN];
//	};
	struct thread_data *dataHilo;
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	char directorioNuevo[MSG_LEN];					// Variable para formar un nuevo directorio

	dataHilo = (struct thread_data *) threadarg;
	printf("dataHilo->directory = %s\n",dataHilo->directory );
	directorioTemp = opendir(dataHilo->directory );

	if (directorioTemp == NULL)
	{
		perror("opendir\n");
		exit(-1);
	}

	for (dp = readdir(directorioTemp); dp != NULL; dp = readdir(directorioTemp))
	{
		exists = stat(dp->d_name, &bufferDeArchivo);
		if (exists < 0)
		{
			fprintf(stderr, "%s not found\n", dp->d_name);
		}
		else
		{
			if (S_ISDIR(bufferDeArchivo.st_mode))
			{
				fprintf(archivoSalida,"Directorio: %s\n", dp->d_name);
				if (strcmp(dp->d_name,dataHilo->directory) != 0)
				{
					strcpy(directorioNuevo,"");
					fprintf(archivoSalida,"Directorio Completo: %s/%s\n",dataHilo->directory,dp->d_name);
					fprintf(directorioNuevo,"%s/%s",dataHilo->directory,dp->d_name);
					fprintf(archivoSalida,"directorioNuevo = %s\n",directorioNuevo);
					pushToStack(&pilaDirectorios,directorioNuevo);
				}
			}
			else
			{
				fprintf(archivoSalida,"Fichero: %s Numero De Bloques: %ld\n", dp->d_name,bufferDeArchivo.st_blocks);
				pthread_mutex_lock(&numTotalBlocksLock);
				numTotalBlocks += bufferDeArchivo.st_blocks;
				pthread_mutex_unlock(&numTotalBlocksLock);
			}
		}
	}

	return numTotalBlocks;
}

