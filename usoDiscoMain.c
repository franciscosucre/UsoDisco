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


void *examinarDirectorio(void *threadarg);

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

				rc = pthread_create(&hilos[i], NULL, examinarDirectorio, (void *) &thread_data_array[i]);
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
	printf("archivoSalidaNombre = %s\n\n",archivoSalidaNombre);
	fprintf(archivoSalida,"numTotalBlocks = %d\n",numTotalBlocks);

	if (archivoSalida != NULL)
	{
		fclose(archivoSalida);
	}

	return 0;
}

/*
 * Function:  examinarDirectorio(void *threadarg)
 * --------------------
 *  Funcion utilizada por los hilos para examinar los contenidos de un directorio, el hilo
 *  abre el directorio asignado e iterara entre los contenidos, si el archivo apuntado es
 *  un Archivo Regular o Fichero se calculara el numero de bloques que ocupa y se sumara a
 *  la variable global de numero de bloques, si es un directorio se agregara a la pila de directorios
 *
 *  threadarg: Informacion necesaria para que el hilo creado pueda completar la funcion, contiene:
 *  	thread_id: identificador del hilo
 *  	directory: directorio asignado al hilo para examinar
 *
 *  returns: -1 si tiene un error y 0 si se ejecuto correctamente
 */
void *examinarDirectorio(void *threadarg)
{
	struct thread_data *dataHilo;			// Variable temporal para obtener los argumentos de la funcion
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	char directorioNuevo[MSG_LEN];			// Variable para formar un nuevo directorio

	/* Obtenemos los argumentos de la funcion */
	dataHilo = (struct thread_data *) threadarg;

	/* Si el directorio no tiene '/' lo agregamos para evitar problemas */
	if (dataHilo->directory[strlen(dataHilo->directory) - 1] != '/')
	{
		sprintf(dataHilo->directory,"%s/",dataHilo->directory);
	}

	/* Abrimos el directorio */
	directorioTemp = opendir(dataHilo->directory );

	/* Si no se pudo abrir el directorio la funcion devuelve error */
	if (directorioTemp == NULL)
	{
		printf("opendir\n");
		return (void *) -1;
	}

	/* Iteramos entre los archivos del directorio */
	for (dp = readdir(directorioTemp); dp != NULL; dp = readdir(directorioTemp))
	{
		strcpy(directorioNuevo,"");
		/* Formamos la direccion del archivo que se busca */
		sprintf(directorioNuevo,"%s%s",dataHilo->directory,dp->d_name);
		exists = stat(directorioNuevo, &bufferDeArchivo);

		/* Caso 1: Si no se encuentran los archivos se envia un aviso al archivo de salida */
		if (exists < 0)
		{
			fprintf(archivoSalida,"%s not found\n", directorioNuevo);
		}
		/* Caso 2: Se encuentra el archivo */
		else
		{
			/* Caso 2.1: El archivo es un directorio */
			if (S_ISDIR(bufferDeArchivo.st_mode))
			{
				/* Para evitar ciclos infinitos, solo agregamos archivos sino son el mismo directorio
				 * o el directorio padre */
				if (strcmp(dp->d_name,".") != 0 && strcmp(dp->d_name,"..") != 0)
				{
					/* Empilamos el nuevo  directorio en nuestra pila */
					//fprintf(archivoSalida,"Directorio Completo: %s/%s\n",dataHilo->directory,dp->d_name);
					pushToStack(directorioNuevo);
				}
			}
			/* Caso 2.2: El archivo es un Archvo Regular o Fichero */
			else
			{
				/* Enviamos al archivo de salida el numero de bloques de archivo y su direccion */
				fprintf(archivoSalida,"(%ld)	%s\n", bufferDeArchivo.st_blocks,dp->d_name);
				/* Agregamos a la cuenta total el tamano del archivo */
				pthread_mutex_lock(&numTotalBlocksLock);
				numTotalBlocks += bufferDeArchivo.st_blocks;
				pthread_mutex_unlock(&numTotalBlocksLock);
			}
		}
	}

	/* Marco el hilo como libre */
	liberarHilo(dataHilo->thread_id);

	/* Cierro el directorio */
	closedir(directorioTemp);

	/* Retorno un exito */
	return (void *) 0;
}

/*
 * Function:  liberarHilo(int posicion)
 * --------------------
 *  Marca un hilo en la lista de hilos global como libre para que
 *  se le pueda asignar otro directorio
 *
 *	int posicion: Posicion en la que se encuentra el hilo en la lista de hilos global
 *
 *  returns: void
 */
void liberarHilo(int posicion)
{
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	hilosEstado[posicion] = 0;
	pthread_mutex_unlock(&hilosEstadosBlocksLock);
}

/*
 * Function:  asignarHilo(int posicion)
 * --------------------
 *  Marca un hilo en la lista de hilos global como libre para que
 *  NO se le pueda asignar otro directorio hasta que sea liberado
 *
 *	int posicion: Posicion en la que se encuentra el hilo en la lista de hilos global
 *
 *  returns: void
 */
void asignarHilo(int posicion)
{
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	hilosEstado[posicion] = 1;
	pthread_mutex_unlock(&hilosEstadosBlocksLock);
}

/*
 * Function:  hilosTrabajando()
 * --------------------
 *  Examina la lista de hilos y verifica si hay hilos trabajando
 *  actualmente
 *
 *  returns: (int) retorna 1 si hay hilos trabajando actualmente
 *  y 0 si no hay hilos trabajando actualmente
 */
int hilosTrabajando()
{
	int trabajadores;	// Variable que usaremos para el retorno
	int i;				// Iterador

	/* Inicializamos la variable en el valor por defecto asumiendo que
	 * no hay hilos trabajando */
	trabajadores = 0;

	/* Recorremos el arreglo de estado de los hilos */
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	for (i = 0; i < nivelConcurrencia; i++)
	{
		/* Si encontramos un hilo trabajando salimos del ciclo
		 * con nuestro resultado */
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

/*
 * Function:  initializeStack()
 * --------------------
 *  Inicializa todos los espacios de la pila de directorios con
 *  un string vacio
 *
 *  returns: void
 */
void initializeStack()

{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		strcpy(pilaDirectorios.listaDirectorios[i],"");
	}
}

/*
 * Function:  stackSize()
 * --------------------
 *  Obtiene el numero de directorios actualmente en la pila
 *
 *	int posicion: Posicion en la que se encuentra el hilo en la lista de hilos global
 *
 *  returns: void
 */
int  stackSize()
{
	int i;
	int returnValue;

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	/* Caso 1: La primera posicion esta vacia, por lo que la pila
	 * esta vacia
	 */
	if (strcmp(pilaDirectorios.listaDirectorios[0],"") == 0)
	{
		returnValue = 0;
	}
	/* Caso 2: Hay posiciones no vacias
	 */
	else
	{
		/* Iteramos en la pila */
		for (i = 1; i < MAXDIR; i++)
		{
			/* Cuando conseguimos un string vacio sabemos que hemos llegado al
			 * final de la pila
			 */
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

/*
 * Function:  pushToStack(char directorioNuevo[MSG_LEN])
 * --------------------
 *  Agrega un directorio a la primera posicion disponible en la pila
 *  global de directorios
 *
 *	char directorioNuevo[MSG_LEN]: Directorio nuevo que se agregara
 *	a la pila global de directorios
 *
 *  returns: void
 */
void pushToStack(char directorioNuevo[MSG_LEN])

{
	int i;
	int lastPosition;
	char directoryToReturn[MSG_LEN];

	lastPosition = stackSize(pilaDirectorios);

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	strcpy(pilaDirectorios.listaDirectorios[lastPosition],directorioNuevo);
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);

}

/*
 * Function:  popFromStack(char directoryToReturn[MSG_LEN])
 * --------------------
 *  Agrega un directorio a la primera posicion disponible en la pila
 *  global de directorios
 *
 *	char directorioNuevo[MSG_LEN]: Directorio nuevo que se agregara
 *	a la pila global de directorios
 *
 *  returns: void
 */
void popFromStack(char directoryToReturn[MSG_LEN])
{
	int i;
	int lastPosition;

	lastPosition = stackSize(pilaDirectorios);

	pthread_mutex_lock(&pilaDirectoriosBlocksLock);
	strcpy(directoryToReturn,pilaDirectorios.listaDirectorios[lastPosition - 1]);
	strcpy(pilaDirectorios.listaDirectorios[lastPosition - 1],"");
	pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
}

/*
 * Function:  asignarHilo
 * --------------------
 *  Marca un hilo en la lista de hilos global como libre para que
 *  NO se le pueda asignar otro directorio hasta que sea liberado
 *
 *	int posicion: Posicion en la que se encuentra el hilo en la lista de hilos global
 *
 *  returns: void
 */
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
