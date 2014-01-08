//----------------------------------------------------------------------------------------
// shell.c
// Intérprete de comandos sencillo. Lee e invoca comandos desde la consola. Si los
// comandos comienzan con '&' los ejecutará en segundo plano.
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
	SpaceId newProc;
	char buffer[1024], tmpChar;
	int i;

	while (1) {

		// Mostramos el prompt en pantalla.

		Write(">:", 2, ConsoleOutput);

		// Leemos caracteres desde la consola hasta encontrar el "\n".

		i = 0;

		do {
			Read(&tmpChar, 1, ConsoleInput);
			buffer[i] = tmpChar;
			i++;
		} while (tmpChar != '\n');

		buffer[i-1] = '\0';

		// Definimos si el comando debe ser ejecutado o no en background.

		if (buffer[0] == '&')
		{
			buffer[0] = ' ';
			newProc = Exec(buffer);
		}
		else
		{
			newProc = Exec(buffer);
			Join(newProc);
		}
	}
}
