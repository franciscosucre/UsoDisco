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
#define BASIC_PERMISSIONS 0666
#define N 20

// CONSTANTES

extern const char *mkfifoError;
extern const char *selectError;
extern const char *openError;
extern const char *getcwdError;
extern const char *mallocError;
extern const char *rvError;
extern const char *argNumError;
extern const char *dirNotExistsError;
extern const char *argOrdError;
extern const char *invOrdError;

extern const char *helpMenu;


// FUNCIONES

char* getErrorMessage(const char* errorMessage,int line, char* file);
char* getWord(char* string,char* delimeter,int index);
void writeFull(char *token, char dst[]);
void errorAndExit(const char* errorMessage);


#endif /* MESSAGES_H_ */
