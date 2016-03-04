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
#include <sys/stat.h>			// Stats
#include <time.h>				// Clock


#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "stdout"

struct thread_data
{
   int  thread_id;
   char directory[MSG_LEN];
};

struct directoryStack
{
	char listaDirectorios[MAXDIR][MSG_LEN];
};

FILE * archivoSalida;							// Archivo de salida
int numDirectorios;								// Numero de directorios en la lista
int dirAnalizar;								// Indice del directorio a analizar
char listaDirectorios[MAXDIR][MSG_LEN]; 		// Lista de directorios a analizar
pthread_mutex_t numTotalBlocksLock;
pthread_mutex_t hilosEstadosBlocksLock;
pthread_mutex_t pilaDirectoriosBlocksLock;
int numTotalBlocks;
struct directoryStack pilaDirectorios;
int hilosEstado[MAXDIR];
clock_t start, end;
double cpu_time_used;
struct thread_data thread_data;
struct thread_data thread_data_array[MAXDIR];
int stackSizeTemp;
int nivelConcurrencia;					// Numero de hilos a crear


void *funcHilo(void *threadarg);

void liberarHilo(int posicion);
void asignarHilo(int posicion);
int hilosTrabajando();

void initializeStack();
void pushToStack(char directorioNuevo[MSG_LEN]);
void printStack();
void popFromStack(char directoryToReturn[MSG_LEN]);

int main(int argc, char *argv[])
{
	char directorioInicial[MSG_LEN];		// Directorio a analizar
	char archivoSalidaNombre[MSG_LEN];		// Nombre del Archivo de salida

	int i,j;								// Iteradores
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
        		printf("El directorio inicial es %s\n",directorioInicial);
				directorioTemp = opendir(directorioInicial);

				if (directorioTemp == NULL)
				{
					printf("%s\n",dirNotExistsError);
					exit(-1);
				}
        		dirSwith = 1;
        		closedir(directorioTemp);
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

	initializeStack();

	for (i = 0; i < nivelConcurrencia; i++)
	{
		hilosEstado[i]= 0;
	}

	pushToStack(directorioInicial);

	pthread_mutex_init(&numTotalBlocksLock,NULL);
	pthread_mutex_init(&hilosEstadosBlocksLock,NULL);
	pthread_mutex_init(&pilaDirectoriosBlocksLock,NULL);

	i = 0;

	start = clock();
	int size;
	int trabajando;

	while (1)
	{
		size = stackSize();
		trabajando = hilosTrabajando();

		if ((size==0) && (trabajando == 0))
		{
			break;
		}
		while (size>0)
		{
			if (hilosEstado[i] == 0)
			{
				strcpy(thread_data.directory,"");

				/* tomo un directorio de la pila */
				strcpy(thread_data_array[i].directory,"");
				popFromStack(thread_data_array[i].directory);
				thread_data_array[i].thread_id = i;

				/* Defino el id del hilo */

				thread_data.thread_id = i;

				/* Marco el hilo como ocupado */
				asignarHilo(i);

				rc = pthread_create(&hilos[i], NULL, funcHilo, (void *) &thread_data_array[i]);
				if (rc)
				{
					printf("ERROR; return code from pthread_create() is %d\n", rc);
					exit(-1);
				}

				/* Si la cola esta vacia pero hay hilos trabajando esperamos que traigan informacion de vuelta */
				if ((stackSize()==0) && (hilosTrabajando() > 0))
				{

					/* Retorno De Hilos */
					for(j=0; j<nivelConcurrencia; j++)
					{
						pthread_mutex_lock(&hilosEstadosBlocksLock);
						if (hilosEstado[j] == 1)
						{
							pthread_mutex_unlock(&hilosEstadosBlocksLock);
							rc = pthread_join(hilos[j], &status);
							if (rc)
							{
								printf("ERROR; return code from pthread_join() is %d\n", rc);
								exit(-1);
							}
							break;
						}
						else
						{
							pthread_mutex_unlock(&hilosEstadosBlocksLock);
						}
					}

				}
			}
			i = i + 1;

			if (i == nivelConcurrencia)
			{
				i = 0;
			}

			break;
		}

		printf("sigo aqui\n");

	}

	/* Retorno De Hilos */
	for(i=0; i<nivelConcurrencia; i++)
	{
		if (hilosEstado[i] == 1)
		{
			rc = pthread_join(hilos[i], &status);
			if (rc)
			{
				printf("ERROR; return code from pthread_join() is %d\n", rc);
			}
		}
	}

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("\n");
	printf("Tiempo total fue de -->  %e\n",cpu_time_used);
	printf("nivelConcurrencia = %d\n",nivelConcurrencia);
	printf("directorio = %s\n",directorioInicial);
	printf("archivoSalidaNombre = %s\n",archivoSalidaNombre);
	fprintf(archivoSalida,"numTotalBlocks = %d\n",numTotalBlocks);

	if (archivoSalida != NULL)
	{
		fclose(archivoSalida);
	}

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
	struct thread_data *dataHilo;			//
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	char directorioNuevo[MSG_LEN];			// Variable para formar un nuevo directorio

	dataHilo = (struct thread_data *) threadarg;
	directorioTemp = opendir(dataHilo->directory );

	if (directorioTemp == NULL)
	{
		printf("opendir\n");
		return (void *) -1;
	}

	for (dp = readdir(directorioTemp); dp != NULL; dp = readdir(directorioTemp))
	{
		strcpy(directorioNuevo,"");
		if (directorioNuevo[strlen(directorioNuevo) - 1] != "/")
		{
			sprintf(directorioNuevo,"%s/",directorioNuevo);
		}
		sprintf(directorioNuevo,"%s%s",dataHilo->directory,dp->d_name);
		exists = stat(directorioNuevo, &bufferDeArchivo);
		if (exists < 0)
		{
			fprintf(archivoSalida,"%s not found\n", directorioNuevo);
		}
		else
		{
			if (S_ISDIR(bufferDeArchivo.st_mode))
			{
				if (strcmp(dp->d_name,".") != 0 && strcmp(dp->d_name,"..") != 0)
				{
					fprintf(archivoSalida,"HILO: Directorio Completo: %s/%s\n",dataHilo->directory,dp->d_name);
					pushToStack(directorioNuevo);
				}
			}
			else
			{
				fprintf(archivoSalida,"HILO: Fichero: %s Numero De Bloques: %ld\n", dp->d_name,bufferDeArchivo.st_blocks);
				pthread_mutex_lock(&numTotalBlocksLock);
				numTotalBlocks += bufferDeArchivo.st_blocks;
				pthread_mutex_unlock(&numTotalBlocksLock);
			}
		}
	}

	/* Marco el hilo como libre */
	liberarHilo(dataHilo->thread_id);

	closedir(directorioTemp);

	return (void *) 0;
}

void liberarHilo(int posicion)
{
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	hilosEstado[posicion] = 0;
	pthread_mutex_unlock(&hilosEstadosBlocksLock);
}

void asignarHilo(int posicion)
{
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	hilosEstado[posicion] = 1;
	pthread_mutex_unlock(&hilosEstadosBlocksLock);
}

int hilosTrabajando()
{
	int trabajadores;
	int i;

	trabajadores = 0;


	pthread_mutex_lock(&hilosEstadosBlocksLock);
	for (i = 0; i < nivelConcurrencia; i++)
	{
		if (hilosEstado[i]== 1)
		{
			trabajadores = 1;
			break;
		}
	}
	pthread_mutex_unlock(&hilosEstadosBlocksLock);

	return trabajadores;
}

/* PILA DE DIRECTORIOS */

void initializeStack()

{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		strcpy(pilaDirectorios.listaDirectorios[i],"");
	}
	printf("Inicialize el stack\n");
}

int  stackSize()
{
	int i;
	int returnValue;

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	if (strcmp(pilaDirectorios.listaDirectorios[0],"") == 0)
	{
		returnValue = 0;
	}
	else
	{
		for (i = 1; i < MAXDIR; i++)
		{
			if (strcmp(pilaDirectorios.listaDirectorios[i],"") == 0)
			{
				returnValue = i;
				break;
			}
		}
	}
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);

	return returnValue;


}

void pushToStack(char directorioNuevo[MSG_LEN])

{
	int i;
	int lastPosition;
	char directoryToReturn[MSG_LEN];

	lastPosition = stackSize(pilaDirectorios);

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	strcpy(pilaDirectorios.listaDirectorios[lastPosition],directorioNuevo);
	printf("Push To Stack -----> %s\n",directorioNuevo);
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);

}

void popFromStack(char directoryToReturn[MSG_LEN])
{
	int i;
	int lastPosition;

	lastPosition = stackSize(pilaDirectorios);

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	strcpy(directoryToReturn,pilaDirectorios.listaDirectorios[lastPosition - 1]);
	strcpy(pilaDirectorios.listaDirectorios[lastPosition - 1],"");
	printf("Pop from Stack -----> %s\n",directoryToReturn);
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
}

void printStack()
{
	int i;
	int lastPosition;

	lastPosition = stackSize(pilaDirectorios);

	printf("\n\n----- ESTADO ACTUAL DE LA PILA----- \n\n");

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	if (strcmp(pilaDirectorios.listaDirectorios[i],"") == 0)
	{
		printf("Pila Vacia!\n\n");
	}
	else
	{
		for (i = 0; i < lastPosition; i++)
		{
			printf("elemento %d de la pila --> %s\n",i,pilaDirectorios.listaDirectorios[i]);
		}
	}
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);

}
