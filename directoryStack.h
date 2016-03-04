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

struct directoryStack
{
	char listaDirectorios[MAXDIR][MSG_LEN];
};


#endif /* DIRECTORYSTACK_H_ */
