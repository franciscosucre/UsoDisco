/*
 * pthreadQueue.h
 *
 *  Created on: Mar 2, 2016
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

#ifndef PTHREADQUEUE_H_
#define PTHREADQUEUE_H_

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "None"

struct pThreadQueue
{
	pthread_t *queque[MAXDIR];
}ThreadQueue;

void initializeQueue(struct pThreadQueue* threadQueue);
void Enqueue(struct pThreadQueue* threadQueue,pthread_t* newThread);
pthread_t* Dequeue(struct pThreadQueue* threadQueue);
int  queueSize(struct pThreadQueue* threadQueue);

#endif /* PTHREADQUEUE_H_ */
