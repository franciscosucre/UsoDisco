/*
 * usoDiscoMain.c
 *
 *  Created on: Feb 25, 2016
 *      Author: francisco
 */

#include "messages.h"         	// Mensajes De Error y funciones comunes
#include <unistd.h>				// getcwd

#define MSG_LEN 500             // NOTA: VER EN CUANTO SE DEJARA ESTE ARREGLO
#define NAME_LEN 50

int main(int argc, char *argv[])
{
	char directorio[MSG_LEN];			// Directorio a analizar
	char archivoSalida[MSG_LEN];		// Archivo de salida
	int nivelConcurrencia;				// Numero de hilos a crear
	int i;								// Iterador

	/* Cargamos los valores por defecto de los argumentos */

	archivoSalida = NULL;
	nivelConcurrencia = 1;

	directorio = getcwd(directorio, sizeof(directorio));
	if (getcwd(directorio, sizeof(directorio)) != NULL)
	{
		fprintf(stdout, "Current working dir: %s\n", directorio);
	}
	else
	{
		errorAndExit(getErrorMessage(getcwdError,__LINE__, __FILE__));
	}

	/* Caso 1: Se recibio 1 solo argumento */
	if (argc == 2)
	{
		if ((strcmp(argv[i],"-h")) == 0)
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
	else if (argc > 2 && argc <= 7)
	{
        for (i=0; i<argc; i = i + 2)
        {
        	if ((strcmp(argv[i],"-n")) == 0)
			{
        		nivelConcurrencia = argv[i + 1];
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

}
