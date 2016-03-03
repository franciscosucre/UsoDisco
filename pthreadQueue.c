/*
 * usoDiscoMain.c
 *
 *  Created on: Feb 25, 2016
 *      Author: francisco
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include "pthreadQueue.h"         	// Mensajes De Error y funciones comunes
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

void initializeQueue(struct pThreadQueue* threadQueue);
void Enqueue(struct pThreadQueue* threadQueue,pthread_t* newThread);
pthread_t* Dequeue(struct pThreadQueue* threadQueue);
int  queueSize(struct pThreadQueue* threadQueue);

void initializeQueue(struct pThreadQueue* threadQueue)

{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		threadQueue->queque[i] = NULL;
	}

}

void Enqueue(struct pThreadQueue* threadQueue,pthread_t* newThread)

{
	int lastPosition;
	lastPosition = queueSize(threadQueue);

	threadQueue->queque[lastPosition] = newThread;

}

pthread_t* Dequeue(struct pThreadQueue* threadQueue)
{
	int i;
	int lastPosition;
	pthread_t *threadToReturn;

	lastPosition = queueSize(threadQueue);

	threadToReturn = threadQueue->queque[0];

	for (i = 0; i < lastPosition; i++)
	{
		threadQueue->queque[i] = threadQueue->queque[i + 1];
	}

	return threadToReturn;
}

int  queueSize(struct pThreadQueue* threadQueue)
{
	int i;
	for (i = 0; i < MAXDIR; i++)
	{
		if (threadQueue->queque[i] == NULL)
		{
			return i - 1;
		}
	}
	return MAXDIR;

}
