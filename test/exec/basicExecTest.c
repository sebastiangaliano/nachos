//----------------------------------------------------------------------------------------
// basicExecTest.c
// Programa para comprobar el funcionamiento basico de la syscall exec.
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
	Write("[MAIN]: Executing simpleProg1...\n", 33, ConsoleOutput);

	SpaceId progId = Exec("../test/exec/simpleProg1");

	Write("[MAIN]: Waiting for simpleProg1 to finish...\n", 45, ConsoleOutput);
	Join(progId);

	Write("[MAIN]: simpleProg1 done!\n", 26, ConsoleOutput);

	// Realizamos una llamada a halt para finalizar Nachos.

	Halt();
}
