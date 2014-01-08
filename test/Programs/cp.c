//----------------------------------------------------------------------------------------
// cp.c
// Programa utilitario simple para copiar archivos.
//
// NOTE: for some reason, user programs with global data structures sometimes haven't
// worked in the Nachos environment.  So be careful out there! One option is to allocate
// data structures as automatics within a procedure, but if you do this, you have to
// be careful to allocate a big enough stack to hold the automatics!
//----------------------------------------------------------------------------------------
// Created by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "syscall.h"


int main(int argc, char **argv)
{
	// Controlamos la cantidad de argumentos.

	if (argc < 2)
	{
		Write("[cp]: ERROR: Invalid number of arguments!\n", 42, ConsoleOutput);
		Exit(1);
	}

	// XXX: Suponemos que src y dest son paths completos de un archivo.
	// Obtenemos los argumentos: archivo origen y destino.

	char *src = argv[0];
	char *dest = argv[1];

	// Abrimos el archivo origen.

	OpenFileId srcID = Open(src);

	if (srcID < 0)
	{
		Write("[cp]: ERROR: Can't open the source file!\n", 41, ConsoleOutput);
		Exit(1);
	}

	// Creamos el archivo destino donde copiaremos el origen, y luego lo abrimos.

	Create(dest);
	OpenFileId destID = Open(dest);

	if (destID < 0)
	{
		Write("[cp]: ERROR: Can't open the destiny file!\n", 42, ConsoleOutput);
		Exit(1);
	}

	// Realizamos la copia del contenido del archivo origen al archivo destino.

	char tmpChar;
	int charsReaded;

	do
	{
		charsReaded = Read(&tmpChar, 1, srcID);

		if (charsReaded > 0)
			Write(&tmpChar, 1, destID);
		else
			break;

	} while (1);

	// Cerramos los archivos intervinientes.

	Close(srcID);
	Close(destID);

	// Realizamos una llamada a Exit() para finalizar.

	Exit(0);
}
