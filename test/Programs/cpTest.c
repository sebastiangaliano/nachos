//----------------------------------------------------------------------------------------
// cpTest.c
// Programa para comprobar el funcionamiento basico del comando cp.
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
	Write("[MAIN]: Executing cp...\n", 24, ConsoleOutput);

	SpaceId progId = Exec("../test/Programs/cp \
                           /home/gulinha/Escritorio/README \
                           /home/gulinha/Escritorio/README2");

	Write("[MAIN]: Waiting for cp to finish...\n", 36, ConsoleOutput);
	Join(progId);

	Write("[MAIN]: cp done!\n", 17, ConsoleOutput);

	// Realizamos una llamada a halt para finalizar Nachos.

	Halt();
}
