/*
 * directoryStack.H
 *
 *  Created on: Mar 2, 2016
 *      Author: francisco
 */

#ifndef DIRECTORYSTACK_H_
#define DIRECTORYSTACK_H_

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50
#define MAXDIR 40
#define DEFAULT_CONCURRENCY 1
#define DEFAULT_FILE "None"

struct directoryStack
{
	char listaDirectorios[MAXDIR][MSG_LEN];
};

void initializeStack(struct directoryStack* threadQueue);
void pushToStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN]);
void popFromStack(struct directoryStack* stack,char directorioNuevo[MSG_LEN]);
int  stackSize(struct directoryStack* stack);

#endif /* DIRECTORYSTACK_H_ */
