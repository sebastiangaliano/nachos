//----------------------------------------------------------------------------------------
// fileTest.c
// Test para comprobar las llamadas al sistema que actuan sobre archivos.
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
	char tmpChar, *toConsoleStr;
	int i = 0;

	// Creamos un archivo de nombre "testFile.txt".

	Create("../test/filesystem/testFile.txt");

	// Abrimos el archivo.

	OpenFileId fileId = Open("../test/filesystem/testFile.txt");

	// Solicitamos texto para escribir sobre el archivo.

	toConsoleStr = "Ingrese un texto:\n";
	Write(toConsoleStr, 18, ConsoleOutput);

	do {
		Read(&tmpChar, 1, ConsoleInput);
		Write(&tmpChar, 1, fileId);
		i++;
	} while (tmpChar != '\n');

	// Cerramos el archivo.

	Close(fileId);

	// Abrimos el archivo nuevamente, leemos el texto recien ingresado y lo
	// mostramos en la consola.

	char fromFileStr[i + 1];

	fileId = Open("../test/filesystem/testFile.txt");
	Read(fromFileStr, i, fileId);
	Close(fileId);

	toConsoleStr = "Texto ingresado en el archivo:\n";
	Write(toConsoleStr, 31, ConsoleOutput);
	Write(fromFileStr, i, ConsoleOutput);

	// Realizamos una llamada a halt para finalizar NachOs.

	Halt();
}
