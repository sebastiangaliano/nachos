//----------------------------------------------------------------------------------------
// cat.c
// Programa utilitario simple para mostrar por consola el contenido de un archivo.
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

	if (argc < 1)
	{
		Write("[cat]: ERROR: Invalid number of arguments!\n", 43, ConsoleOutput);
		Exit(1);
	}

	// XXX: Suponemos que file es el path completo del archivo.
	// Obtenemos el argumento: file.

	char *file = argv[0];

	// Abrimos el archivo.

	OpenFileId srcID = Open(file);

	if (srcID < 0)
	{
		Write("[cat]: ERROR: Can't open the source file!\n", 42, ConsoleOutput);
		Exit(1);
	}

	// Mostramos el contenido del archivo por consola.

	char tmpChar;
	int charsReaded;

	do
	{
		charsReaded = Read(&tmpChar, 1, srcID);

		if (charsReaded > 0)
			Write(&tmpChar, 1, ConsoleOutput);
		else
			break;

	} while (1);

	//Write('\n', 1, ConsoleOutput);

	// Cerramos los archivos intervinientes.

	Close(srcID);

	// Realizamos una llamada a Exit() para finalizar.

	Exit(0);
}
