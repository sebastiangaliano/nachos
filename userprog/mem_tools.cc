//----------------------------------------------------------------------------------------
// mem_tools.cc
// Provee funciones de utileria para intercambiar datos entre el espacio de memoria del
// nucleo de Nachos y el espacio de memoria virtual de usuario.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include <system.h>
#include <machine.h>
#include <mem_tools.h>


//----------------------------------------------------------------------------------------
// El siguiente metodo lee un string desde el espacio de memoria virtual de usuario
// <usrAddr> y lo almacena en <outStr>.
//----------------------------------------------------------------------------------------

void readStrFromUsr(int usrAddr, char *outStr)
{
	char c;
	int aux;
	int i = 0;

	do {
		if(!machine->ReadMem(usrAddr + i, 1, &aux))
			ASSERT(machine->ReadMem(usrAddr + i, 1, &aux));

		c = (char) aux;
		outStr[i] = c;
		i = i + 1;
	} while (c != '\0');

	return;
}

//----------------------------------------------------------------------------------------
// El siguiente metodo escribe el string <str> en el espacio de memoria virtual de
// usuario <usrAddr>.
//----------------------------------------------------------------------------------------

void writeStrToUsr(char *str, int usrAddr)
{
	char c;
	int i = 0;

	do {
		c = str[i];
		machine->WriteMem(usrAddr + i, 1, c);
		i = i + 1;
	} while (c != '\0');

	return;
}

//----------------------------------------------------------------------------------------
// El siguiente metodo lee <byteCount> bytes comenzando desde el espacio de memoria
// virtual de usuario <usrAddr> y lo almacena en <outBuff>.
//----------------------------------------------------------------------------------------

void readBuffFromUsr(int usrAddr, char *outBuff, int byteCount)
{
	int i;
	int aux;

	for (i = 0; i < byteCount; i++)
	{
		if (!machine->ReadMem(usrAddr + i, 1, &aux))
			ASSERT(machine->ReadMem(usrAddr + i, 1, &aux));

		outBuff[i] = (char) aux;
	}

	return;
}

//----------------------------------------------------------------------------------------
// El siguiente metodo escribe <byteCount> bytes de <Buff> en el espacio de memoria
// virtual de usuario <usrAddr>.
//----------------------------------------------------------------------------------------

void writeBuffToUsr(char *Buff, int usrAddr, int byteCount)
{
	int i;

	for (i = 0; i < byteCount; i++) {
		machine->WriteMem(usrAddr + i, 1, (int) Buff[i]);
	}

	return;
}

//----------------------------------------------------------------------------------------
// El siguiente metodo calcula la longitud de un string almacenado en el espacio de
// memoria virtual de usuario <usrAddr>.
//----------------------------------------------------------------------------------------

int getStrLenFromUsr(int usrAddr)
{
	char c;
	int aux;
	int i = 0;

	do {
		if(!machine->ReadMem(usrAddr + i, 1, &aux))
			ASSERT(machine->ReadMem(usrAddr + i, 1, &aux));

		c = (char) aux;
		i = i + 1;
	} while ( c != '\0');

	return (i - 1);
}
