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

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50

int main(int argc, char *argv[])
{
	char directorio[MSG_LEN];			// Directorio a analizar
	char archivoSalida[MSG_LEN];		// Archivo de salida
	int nivelConcurrencia;				// Numero de hilos a crear
	int i;								// Iterador
	char* cwd;							// Apuntador utilizado para obtener el directorio actual

	nivelConcurrencia = 1;
	strcpy(archivoSalida,"");
	cwd = getcwd(directorio, sizeof(directorio));
	if ((cwd) != NULL)
	{
		fprintf(stdout, "Directorio Actual: %s\n", directorio);
	}
	else
	{
		errorAndExit(getErrorMessage(getcwdError,__LINE__, __FILE__));
	}

	/* Caso 1: Se recibio 1 solo argumento */
	if (argc == 2)
	{
		if ((strcmp(argv[1],"-h")) == 0)
		{
			printf("%s", helpMenu);
			exit(0);
		}

		else
		{
			printf("%s", invOrdError);
			exit(0);
		}
	}

	/* Caso 2: Se recibieron varios argumentos */
	else if (argc == 3 || argc == 5 || argc == 7)
	{
        for (i=1; i<argc; i = i + 2)
        {
        	if ((strcmp(argv[i],"-n")) == 0)
			{
        		nivelConcurrencia = atoi(argv[i + 1]);
			}

        	else if ((strcmp(argv[i],"-d")) == 0)
			{
        		strcpy(directorio,argv[i + 1]);
			}

        	else if ((strcmp(argv[i],"-o")) == 0)
			{
        		strcpy(archivoSalida,argv[i + 1]);
			}

    		else
    		{
    			printf("%s", invOrdError);
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

	printf("\n");
	printf("nivelConcurrencia = %d\n",nivelConcurrencia);
	printf("directorio = %s\n",directorio);
	printf("archivoSalida = %s\n",archivoSalida);

	return 0;
}
