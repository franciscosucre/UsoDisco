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
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "None"

struct directoryStack
{
	char listaDirectorios[MAXDIR][MSG_LEN];
};

void pushToStack(directoryStack* stack,char directorioNuevo[MSG_LEN])

{


}

char directorioNuevo[MSG_LEN] popFromStack(directoryStack* stack)
{

}

int  stackSize(directoryStack* stack)
{

}
