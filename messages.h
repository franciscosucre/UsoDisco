/*
 * messages.h
 *
 *	Archivo en el cual se encuentran los mensajes de error y funciones utilizadas
 *	para su manejo
 *
 *  Created on: Jan 30, 2016
 *      Author: francisco
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#define MSG_LEN 500
#define MENU_LEN 800
#define ERROR_LEN 100
#define BASIC_PERMISSIONS 0666
#define N 20

// CONSTANTES
extern const char pthreadCreateError[ERROR_LEN];
extern const char pthreadJoinError[ERROR_LEN];
extern const char mkfifoError[ERROR_LEN];
extern const char selectError[ERROR_LEN];
extern const char openError[ERROR_LEN];
extern const char openDirError[ERROR_LEN];
extern const char getcwdError[ERROR_LEN];
extern const char mallocError[ERROR_LEN];
extern const char rvError[ERROR_LEN];
extern const char argNumError[ERROR_LEN];
extern const char dirNotExistsError[ERROR_LEN];
extern const char argOrdError[ERROR_LEN];
extern const char invOrdError[ERROR_LEN];

extern const char helpMenu[MENU_LEN];


// FUNCIONES

void errorAndExit(const char* errorMessage);


#endif /* MESSAGES_H_ */
