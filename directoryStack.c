/*
 * usoDiscoMain.c
 *
 *  Created on: Feb 25, 2016
 *      Author: francisco
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include "directoryStack.h"         	// Mensajes De Error y funciones comunes
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

void initializeStack(struct directoryStack* threadQueue);
void pushToStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN]);
void popFromStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN]);
int  stackSize(struct directoryStack* stack);

void initializeStack(struct directoryStack* stack)

{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		strcpy(stack->listaDirectorios[i],"");
	}
	printf("Inicialize el stack\n");
}

void pushToStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN])

{
	int i;
	int lastPosition;
	char directoryToReturn[MSG_LEN];

	lastPosition = stackSize(stack);

	strcpy(stack->listaDirectorios[lastPosition],directorioNuevo);

}

void popFromStack(struct directoryStack* stack,char directoryToReturn[MSG_LEN])
{
	int i;
	int lastPosition;

	lastPosition = stackSize(stack);

	strcpy(directoryToReturn,stack->listaDirectorios[lastPosition - 1]);
	strcpy(stack->listaDirectorios[lastPosition - 1],"");
}

int  stackSize(struct directoryStack* stack)
{
	int i;

	if (strcmp(stack->listaDirectorios[0],"") == 0)
	{
		return 0;
	}
	else
	{
		for (i = 1; i < MAXDIR; i++)
		{
			if (strcmp(stack->listaDirectorios[i],"") == 0)
			{
				return i;
			}
		}
	}

	return MAXDIR;

}
