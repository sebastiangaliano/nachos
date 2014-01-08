//----------------------------------------------------------------------------------------
// consoleTest.c
// Test para comprobar las llamadas al sistema que actuan sobre la consola.
//
// NOTE: for some reason, user programs with global data structures sometimes haven't
// worked in the Nachos environment.  So be careful out there! One option is to allocate
// data structures as automatics within a procedure, but if you do this, you have to
// be careful to allocate a big enough stack to hold the automatics!
//----------------------------------------------------------------------------------------
// Created by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "syscall.h"


int main()
{
	// Escribimos un texto indicativo sobre la consola.

	char *writeStr = "Ingrese un texto:\n";
	Write(writeStr, 18, ConsoleOutput);

	// Leemos caracteres desde la consola hasta encontrar el "\n".

	char readStr[1024], tmpChar;
	int i = 0;

	do {
		Read(&tmpChar, 1, ConsoleInput);
		readStr[i] = tmpChar;
		i++;
	} while (tmpChar != '\n');

	// Escribimos los caracteres ingresados en la consola.

	writeStr = "Usted ingreso:\n";
	Write(writeStr, 16, ConsoleOutput);
	Write(readStr, i, ConsoleOutput);

	// Realizamos una llamada a halt para finalizar Nachos.

	Halt();
}
