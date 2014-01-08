//----------------------------------------------------------------------------------------
// simpleProg2.c
// Programa simple con motivos de poder comprobar el funcionamiento de la syscall exec.
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
	int i;

	for (i = 1; i <= 5; i++)
	{
		SpaceId progId = Exec("../test/exec/simpleProg3");
		Join(progId);
	}

	// Realizamos una llamada a exit para finalizar el programa.

	Exit(0);
}
