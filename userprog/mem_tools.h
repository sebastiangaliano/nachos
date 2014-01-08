//----------------------------------------------------------------------------------------
// mem_tools.h
// Provee funciones de utileria para intercambiar datos entre el espacio de memoria del
// nucleo de Nachos y el espacio de memoria virtual de usuario.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


// El siguiente metodo permite leer un string de la memoria virtual de un usuario.

void readStrFromUsr(int usrAddr, char *outStr);

// El siguiente metodo permite escribir un string en la memoria virtual de un usuario.

void writeStrToUsr(char *str, int usrAddr);

// El siguiente metodo permite leer una cantidad determinada de bytes desde la memoria
// virtual de un usuario.

void readBuffFromUsr(int usrAddr, char *outBuff, int byteCount);

// El siguiente metodo permite escribir una cantidad determinada de bytes en la memoria
// virtual de un usuario.

void writeBuffToUsr(char *Buff, int usrAddr, int byteCount);

// El siguiente metodo calcula la longitud de un string almacenado en el espacio de
// memoria virtual de usuario <usrAddr>.

int getStrLenFromUsr(int usrAddr);
