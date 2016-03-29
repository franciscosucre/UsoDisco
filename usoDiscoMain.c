/*
 * usoDiscoMain.c
 *
 *  Programa que calcula la cantidad de bloques total ocupados por un directorio, implementado
 *  con hilos para trabajar en concurrencia
 *
 *  Parametros:
 *
 *  	-n <NivelConcurrencia>: Numero de hilos creados para el calculo, si no se da este parametro
 *  	se utilizara 1 hilo
 *  	-o <archivo>: Archivo en el cual se imprimira toda la salida del programa, como los archivos
 *  	y los bloques que ocupa cada uno
 *  	-d <directorio>: Directorio que se va a analizar, si no se da este parametro se analizara
 *  	el directorio en el que se encuentra el ejecutable
 *
 *  Created on: Feb 25, 2016
 *      Author: Francisco Sucre y Midaysa Palacios
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include <unistd.h>				// getcwd
#include <string.h>             // strlen
#include <stdio.h>              // printf
#include <stdlib.h>				// exit
#include <pthread.h>			// Hilos
#include <dirent.h>				// dirent
#include <sys/types.h>			// types
#include <sys/stat.h>			// Stats


#define MSG_LEN 500             // Longitud Maxima de los strings (Mensajes, directorios, etc)
#define MAXDIR 170				// Maximo numero de directorios en la pila
#define MAXTHREADS 20			// Maximo numero de hilos
#define DEFAULT_CONCURRENCY 1	// Numero de hilos por defecto
#define DEFAULT_FILE stdout		// Salida por defecto
#define DEFAULT_DIR "."			// Directorio por defecto

/* Pila de directorios */
struct directoryStack
{
	char listaDirectorios[MAXDIR][MSG_LEN];
};


/* Estructura que guarda los argumentos a ser utilizados por los hilos */
struct thread_data
{
   int  thread_id;
   char directory[MSG_LEN];
};

FILE * archivoSalida;								// Archivo de salida
int numDirectorios;									// Numero de directorios en la lista
int dirAnalizar;									// Indice del directorio a analizar
char listaDirectorios[MAXDIR][MSG_LEN]; 			// Lista de directorios a analizar
pthread_mutex_t numTotalBlocksLock;					// Mutex para asegurar la exclusividad mutua con el contador
													// de bloques global
pthread_mutex_t hilosEstadosBlocksLock;				// Mutex para asegurar la exclusividad mutua con el arreglo
													// de estado de hilos global
pthread_mutex_t pilaDirectoriosBlocksLock;			// Mutex para asegurar la exclusividad mutua con la pila
													// global de directorios
int numTotalBlocks;									// Variable global para la cuenta de bloques
struct directoryStack pilaDirectorios;				// Pila Global de directorios
int hilosEstado[MAXTHREADS];							// Lista de estado de los hilos, indican si estan libres o
													// trabajando
struct thread_data thread_data_array[MAXTHREADS];		// Variable que guarda los argumentos a ser utilizados por los hilos
int nivelConcurrencia;								// Numero de hilos a crear

struct directoryStack pilaDirectoriosHilo[MAXDIR];	// Arreglo de pilas de directorio en las que los hilos guardaran
													// los directorios que encuentren
int bloquesHilos[MAXTHREADS];							// Arreglo de enteros en el que los hilos guardaran sus cuentas
													// de bloques

/* ---------------FUNCIONES--------------- */

void *examinarDirectorio(void *threadarg);

/* Manejo De Hilos */
void liberarHilo(int posicion);
void asignarHilo(int posicion);
void esperarHilo(int posicion);
int hilosTrabajando();
int hilosEsperando();

/* Manejo de pila de directorios */
void initializeStack(struct directoryStack* stack);
void pushToStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN]);
void printStack(struct directoryStack* stack);
void popFromStack(struct directoryStack* stack,char directoryToReturn[MSG_LEN]);

int main(int argc, char *argv[])
{
	char directorioInicial[MSG_LEN];		// Directorio a analizar
	char archivoSalidaNombre[MSG_LEN];		// Nombre del Archivo de salida

	int i,j,z;								// Iteradores
	char* cwd;								// Apuntador utilizado para obtener el directorio actual
	int salidaSwitch,levelSwitch,				// Variables booleanas que indican si ya se dio un argumento
	dirSwitch;
	int rc;									//
	void *status;							//
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	int bloquesTotal;						// Numero de bloques en total
	pthread_t hilos[MAXTHREADS];				// Lista de hilos

	int sizePilaHilo;						// Variable auxiliar para guardar el tamano de una pila retornada
											// por un hilo
	int size;								// Variable auxiliar para guardar el tamano de la pila global
	char directorioNuevo[MSG_LEN];			// Variable auxiliar para guardar directorios

	int libres;								// Variable booleana que indica si hay hilos libres
	int trabajando;							// Variable booleana que indica si hay hilos trabajando
	int esperando;							// Variable booleana que indica si hay hilos esperando


	/* Inicializamos los Switches */
	salidaSwitch = 0;
	levelSwitch = 0;
	dirSwitch = 0;

	/* Se asignan los valores por defectos */
	nivelConcurrencia = DEFAULT_CONCURRENCY;
	archivoSalida = DEFAULT_FILE;
	strcpy(directorioInicial,DEFAULT_DIR);

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
			errorAndExit(invOrdError);
		}
	}

	/* Caso 2: Se recibieron varios argumentos */
	else if (argc == 3 || argc == 5 || argc == 7)
	{
		/* Recorremos la lista de argumentos */
        for (i=1; i<argc; i = i + 2)
        {
        	/* Caso 2.1: Se recibio el argumento del numero de concurrencia */
        	if ((strcmp(argv[i],"-n")) == 0 && levelSwitch == 0)
			{
        		nivelConcurrencia = atoi(argv[i + 1]);
        		if (nivelConcurrencia >= MAXTHREADS)
        		{
        			printf("%s\n\n",maxThreadError);
        			exit(-1);
        		}
        		levelSwitch = 1;
			}
        	/* Caso 2.2: Se recibio el argumento del numero de concurrencia */
        	else if ((strcmp(argv[i],"-d")) == 0 && dirSwitch == 0)
			{
        		strcpy(directorioInicial,argv[i + 1]);
				directorioTemp = opendir(directorioInicial);

				if (directorioTemp == NULL)
				{
					errorAndExit(dirNotExistsError);
				}
        		dirSwitch = 1;
        		closedir(directorioTemp);
			}
        	/* Caso 2.3: Se recibio el argumento del numero de concurrencia */
        	else if ((strcmp(argv[i],"-o")) == 0 && salidaSwitch == 0)
			{
        		strcpy(archivoSalidaNombre,argv[i + 1]);
        		archivoSalida = fopen (archivoSalidaNombre, "w+");
        		salidaSwitch = 1;
			}
        	/* Caso 2.4: Se recibieron argumentos en un formato invalido */
    		else
    		{
    			errorAndExit(invOrdError);
    		}
        }

	}

	/* Caso 3: Se recibieron mas argumentos de los posibles */
	else if (argc > 7)
	{
		printf("%s", argNumError);
		exit(0);
	}

	/* Inicializamos la pila y la lista de estados de los hilos */
	initializeStack(&pilaDirectorios);

	for (i = 0; i < nivelConcurrencia; i++)
	{
		hilosEstado[i]= 0;
	}

	/* Agregamos el directorio inicial a la pila */
	pushToStack(&pilaDirectorios,directorioInicial);

	/* Inicializamos los Mutex */
	pthread_mutex_init(&numTotalBlocksLock,NULL);
	pthread_mutex_init(&hilosEstadosBlocksLock,NULL);
	pthread_mutex_init(&pilaDirectoriosBlocksLock,NULL);

	i = 0;

	while (1)
	{
		/* Calculamos el tamano actual de la pila de directorios global */
		pthread_mutex_lock(&pilaDirectoriosBlocksLock);
		size = stackSize(&pilaDirectorios);
		pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
		/* Verificamos si hay hilos que se encuentran trabajando actualmente */
		trabajando = hilosTrabajando();
		/* Verificamos si hay hilos que se encuentran esperando actualmente */
		esperando = hilosEsperando();
		/* Verificamos si hay hilos que se encuentran libres actualmente */
		libres = hiloslibres();

		/* Si no hay hilos trabajando y no hay directorios para examinar
		 * entonces el programa termina
		 */
		if ((size==0) && (trabajando == 0) && (esperando == 0))
		{
			break;
		}

		/* si hay hilos esperando recibimos sus respuestas */
		if (esperando > 0)
		{
			/* Recorremos los hilos */
			for(j=0; j<nivelConcurrencia; j++)
			{
				pthread_mutex_lock(&hilosEstadosBlocksLock);
				/* Si el hilo esta esperando obtenemos su respuesta */
				if (hilosEstado[j] == 2)
				{
					/* Esperamos a que termine su ejecucion */
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
					rc = pthread_join(hilos[j], &status);
					if (rc)
					{
						errorAndExit(pthreadJoinError);
					}
					/* Obtenemos la cuenta calculada por el hilo y se la sumamos a la global */
					numTotalBlocks += bloquesHilos[j];
					/* Calculamos el tamano de la pila que retorno el hilo */
					sizePilaHilo = stackSize(&pilaDirectoriosHilo[j]);
					/* Extraemos los directorios nuevos del hilo y los agregamos a la pila global */
					for(z=0; z<sizePilaHilo; z++)
					{
						popFromStack(&pilaDirectoriosHilo[j],directorioNuevo);
						if (strcmp(directorioNuevo,"") != 0)
						{
							pthread_mutex_lock(&pilaDirectoriosBlocksLock);
							pushToStack(&pilaDirectorios,directorioNuevo);
							pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
						}

					}
					/* Marcamos el hilo como libre */
					liberarHilo(j);
				}
				else
				{
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
				}
			}
		}
		/* Si hay hilos libres y directorios pendientes asignamos todos los que podemos */
		if (size>0 && libres > 0)
		{
			i = 0;
			/* Mientras hayan elementos en la pila seguire asignando hilos libres */
			while (size>0)
			{
				pthread_mutex_lock(&hilosEstadosBlocksLock);
				/* Si encuentro un hilo libre procedo a asignarle un directorio */
				if (hilosEstado[i] == 0)
				{
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
					/* Extraigo un directorio de la pila */
					strcpy(thread_data_array[i].directory,"");
					pthread_mutex_lock(&pilaDirectoriosBlocksLock);
					popFromStack(&pilaDirectorios,thread_data_array[i].directory);
					pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
					size = size -1;

					/* Defino el id del hilo */
					thread_data_array[i].thread_id = i;

					/* Marco el hilo como ocupado */
					asignarHilo(i);
					/* Creamos un hilo nuevo y lo enviamos a examinar n directorio */
					rc = pthread_create(&hilos[i], NULL, examinarDirectorio, (void *) &thread_data_array[i]);
					if (rc)
					{
						errorAndExit(pthreadCreateError);
					}
					/* Si existe un error en el pipe salimos del programa */
					else if (status)
					{
						exit(-1);
					}
				}
				else
				{
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
				}
				/* Seguimos iterando */
				i = i + 1;
				/* Si llegamos al final de la lista iniciamos nuevamente */
				if (i >= nivelConcurrencia)
				{
					i = 0;
				}
				/* Verificamos si hay hilos que se encuentran libres actualmente */
				libres = hiloslibres();
				if (libres == 0)
				{
					break;
				}
			}
		}

		/* Si la cola esta vacia pero hay hilos trabajando esperamos que traigan informacion de vuelta */
		if ((size==0) && (trabajando > 0))
		{

			/* Retorno De Hilos */
			for(j=0; j<nivelConcurrencia; j++)
			{
				pthread_mutex_lock(&hilosEstadosBlocksLock);
				if (hilosEstado[j] == 1)
				{
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
					/* Esperamos al hilo para obtener sus resultados */
					rc = pthread_join(hilos[j], &status);
					if (rc)
					{
						errorAndExit(pthreadJoinError);
					}
					/* Si existe un error en el pipe salimos del programa */
					else if (status)
					{
						exit(-1);
					}
					/* Obtenemos la cuenta calculada por el hilo y se la sumamos a la global */
					pthread_mutex_lock(&numTotalBlocksLock);
					numTotalBlocks += bloquesHilos[j];
					pthread_mutex_unlock(&numTotalBlocksLock);
					/* Calculamos el tamano de la pila que retorno el hilo */
					sizePilaHilo = stackSize(&pilaDirectoriosHilo[j]);
					/* Extraemos los directorios nuevos del hilo y los agregamos a la pila global */
					for(z=0; z<sizePilaHilo; z++)
					{
						popFromStack(&pilaDirectoriosHilo[j],directorioNuevo);
						if (strcmp(directorioNuevo,"") != 0)
						{
							pthread_mutex_lock(&pilaDirectoriosBlocksLock);
							pushToStack(&pilaDirectorios,directorioNuevo);
							pthread_mutex_unlock(&pilaDirectoriosBlocksLock);
						}

					}
					/* Marcamos el hilo como libre */
					liberarHilo(j);
					break;
				}
				else
				{
					pthread_mutex_unlock(&hilosEstadosBlocksLock);
				}

			}

		}
	}

	fprintf(archivoSalida,"numTotalBlocks = %d\n",numTotalBlocks);
	// Print para medir el tiempo
	//fprintf(archivoSalida,"\ncpu_time_used = %f\n",cpu_time_used);

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
	struct directoryStack stackTemp;
	DIR* directorioTemp;					// Variable para almacenar directorios
	struct dirent *dp;						// Estructura utiliada para seleccionar los ficheros al
											// recorrer
	struct stat bufferDeArchivo;			// Estructura utilizada para guardar la informacion
											// de los ficheros o directorios
	int exists;								// Variable que indica si un directorio existe
	char directorioNuevo[MSG_LEN];			// Variable para formar un nuevo directorio

	/* Obtenemos los argumentos de la funcion */
	dataHilo = (struct thread_data *) threadarg;

	initializeStack(&pilaDirectoriosHilo[dataHilo->thread_id]);
	bloquesHilos[dataHilo->thread_id] = 0;

	/* Si el directorio no tiene '/' lo agregamos para evitar problemas */
	if (dataHilo->directory[strlen(dataHilo->directory) - 1] != '/')
	{
		sprintf(dataHilo->directory,"%s/",dataHilo->directory);
	}

	/* Para DEBUG */
	//fprintf(archivoSalida,"Hilo: %d --> %s\n",dataHilo->thread_id,dataHilo->directory);

	/* Abrimos el directorio */
	directorioTemp = opendir(dataHilo->directory );

	/* Si no se pudo abrir el directorio la funcion devuelve error */
	if (directorioTemp == NULL)
	{
		printf("%s\n",openDirError);
		return (void *) -1;
	}

	/* Utilizado para DEBUG */

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
					pushToStack(&pilaDirectoriosHilo[dataHilo->thread_id],directorioNuevo);
				}
				else if (strcmp(dp->d_name,".") == 0)
				{
					/* Agregamos a la cuenta total el tamano del archivo */
					bloquesHilos[dataHilo->thread_id] += bufferDeArchivo.st_blocks;
				}
			}
			/* Caso 2.2: El archivo es un Archvo Regular o Fichero */
			else
			{
				/* Agregamos a la cuenta total el tamano del archivo */
				bloquesHilos[dataHilo->thread_id] += bufferDeArchivo.st_blocks;
			}
		}
	}

	/* Enviamos al archivo de salida el numero de bloques de archivo y su direccion */
	fprintf(archivoSalida,"(%d)	%s\n", bloquesHilos[dataHilo->thread_id],dataHilo->directory);
	/* Cierro el directorio */
	closedir(directorioTemp);
	/* Marco el hilo como esperando */
	esperarHilo(dataHilo->thread_id);
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
 * Function:  esperarHilo(int posicion)
 * --------------------
 *  Marca un hilo en la lista de hilos global como en espera
 *  de esta manera el programa principal sabe que el hilo tiene
 *  un resultado que espera ser recogido
 *
 *	int posicion: Posicion en la que se encuentra el hilo en la lista de hilos global
 *
 *  returns: void
 */
void esperarHilo(int posicion)
{
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	hilosEstado[posicion] = 2;
	pthread_mutex_unlock(&hilosEstadosBlocksLock);
}

/*
 * Function:  hilosLibres()
 * --------------------
 *  Examina la lista de hilos y verifica si hay hilos trabajando
 *  actualmente
 *
 *  returns: (int) retorna 1 si hay hilos trabajando actualmente
 *  y 0 si no hay hilos trabajando actualmente
 */
int hiloslibres()
{
	int libres;	// Variable que usaremos para el retorno
	int i;				// Iterador

	/* Inicializamos la variable en el valor por defecto asumiendo que
	 * no hay hilos trabajando */
	libres = 0;

	/* Recorremos el arreglo de estado de los hilos */
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	for (i = 0; i < nivelConcurrencia; i++)
	{
		/* Si encontramos un hilo trabajando salimos del ciclo
		 * con nuestro resultado */
		if (hilosEstado[i]== 0)
		{
			libres = 1;
			break;
		}
	}
	pthread_mutex_unlock(&hilosEstadosBlocksLock);

	return libres;
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

/*
 * Function:  hilosEsperando()
 * --------------------
 *  Examina la lista de hilos y verifica si hay hilos esperando
 *  actualmente
 *
 *  returns: (int) retorna 1 si hay hilos trabajando actualmente
 *  y 0 si no hay hilos trabajando actualmente
 */
int hilosEsperando()
{
	int esperando;	// Variable que usaremos para el retorno
	int i;				// Iterador

	/* Inicializamos la variable en el valor por defecto asumiendo que
	 * no hay hilos esperando */
	esperando = 0;

	/* Recorremos el arreglo de estado de los hilos */
	pthread_mutex_lock(&hilosEstadosBlocksLock);
	for (i = 0; i < nivelConcurrencia; i++)
	{
		/* Si encontramos un hilo esperando salimos del ciclo
		 * con nuestro resultado */
		if (hilosEstado[i]== 2)
		{
			esperando = 1;
			break;
		}
	}
	pthread_mutex_unlock(&hilosEstadosBlocksLock);

	return esperando;
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
void initializeStack(struct directoryStack* stack)

{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		strcpy(stack->listaDirectorios[i],"");
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
int  stackSize(struct directoryStack* stack)
{
	int i;
	int returnValue;

	/* Iteramos en la pila */
	for (i = 0; i < MAXDIR; i++)
	{
		/* Cuando conseguimos un string vacio sabemos que hemos llegado al
		 * final de la pila
		 */
		if (strcmp(stack->listaDirectorios[i],"") == 0)
		{
			returnValue = i;
			break;
		}
	}
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
void pushToStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN])

{
	int i;
	int lastPosition;
	char directoryToReturn[MSG_LEN];

	/* Calculamos el tamano de la pila para saber en que posicion
	 * esta el ultimo directorio de la pila
	 */
	lastPosition = stackSize(stack);

	if(lastPosition >= MAXDIR - 1)
	{
		printf("%s\n",maxDirError);
		exit(-1);
	}

	strcpy(stack->listaDirectorios[lastPosition],directorioNuevo);

}

/*
 * Function:  popFromStack(char directoryToReturn[MSG_LEN])
 * --------------------
 *  Obtiene el directorio en la ultima posicion de la pila global
 *  de directorios y lo saca de la pila
 *
 *	char directorioNuevo[MSG_LEN]: String donde guardemos el directorio
 *	que vamos a extraer de la pila global de directorios
 *
 *  returns: void
 */
void popFromStack(struct directoryStack* stack,char directoryToReturn[MSG_LEN])
{
	int i;
	int lastPosition;

	/* Calculamos el tamano de la pila para saber en que posicion
	 * esta el ultimo directorio de la pila
	 */
	lastPosition = stackSize(stack);

	/* Obtenemos el directorio */
	strcpy(directoryToReturn,stack->listaDirectorios[lastPosition - 1]);
	/* Lo eliminamos de la pila */
	strcpy(stack->listaDirectorios[lastPosition - 1],"");
}

/*
 * Function:  printStack()
 * --------------------
 *  Imprime los contenidos de la pila global de directorios
 *
 *  UTILIZADO UNICAMENTE PARA BUSCAR ERRORES E INCONSISTENCIAS
 *
 *  returns: void
 */
void printStack(struct directoryStack* stack)
{
	int i;
	int lastPosition;

	lastPosition = stackSize(stack);

	printf("\n\n----- ESTADO ACTUAL DE LA PILA----- \n\n");

	/* Caso 1: La pila esta vacia */
	if (lastPosition == 0)
	{
		printf("Pila Vacia!\n\n");
	}
	/* Caso 2: La pila no esta vacia */
	else
	{
		/* Iteramos en la pila */
		for (i = 0; i < lastPosition; i++)
		{
			printf("elemento %d de la pila --> %s\n",i,stack->listaDirectorios[i]);
		}
	}

}
