// exception.cc
// Entry point into the Nachos kernel from user programs. There are two kinds of things
// that can cause control to transfer back to here from user code:
//
// syscall -- The user code explicitly requests to call a procedure in the Nachos kernel.
// Right now, the only function we support is "Halt".
//
// exceptions -- The user code does something that the CPU can't handle. For instance,
// accessing memory that doesn't exist, arithmetic errors, etc.
//
// Interrupts (which can also cause control to transfer from user code into the Nachos
// kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call. Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "mem_tools.h"
#include "filesys.h"


// Definimos el tamaño predeterminado para los nuevos archivos.

#define NEW_FILE_SIZE 1024

// Definimos el numero maximo de argumentos para la syscall Exec().

#define MAX_ARGS 10

//----------------------------------------------------------------------------------------
// IncreasePC
// El siguiente metodo permite incrementar el program Counter (PCReg).
//
// PCReg (34)		- Current program counter.
// NextPCReg (35)	- Next program counter (for branch delay).
// PrevPCReg (36)	- Previous program counter (for debugging).
//----------------------------------------------------------------------------------------

void IncreasePC()
{
	int regVal;

	regVal = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, regVal);
	regVal = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, regVal);
	regVal += 4;
	machine->WriteRegister(NextPCReg, regVal);
}

//----------------------------------------------------------------------------------------
// RunProcess().
//----------------------------------------------------------------------------------------

void RunProcess(void* dummy)
{
	currentThread->space->RestoreState();	// Load page table register.
	currentThread->space->InitRegisters();	// Set the initial register values.

	machine->Run();							// Jump to the user progam.
	ASSERT(false);							// machine->Run() never returns;
}

//----------------------------------------------------------------------------------------
// ParseCommand().
// El siguiente metodo parsea el string <inputCmd> esperando un patron de la forma
// "bynaryPath arg1 arg2 ... argN". Almacena "binaryPath" en <filePath>, y los
// argumentos "arg1 ... argN" en <argv>.
//----------------------------------------------------------------------------------------

int ParseCommand(char *inputCmd, char **filePath, char **argv)
{
	int argc = 0;
	char *token = NULL;

	// Obtenemos el filePath del binario.

	token = strtok(inputCmd, " \n");

	if (token == NULL)
		return -1;

	*filePath = new char[strlen(token) + 1];
	strncpy(*filePath, token, strlen(token) + 1);

	// Obtenemos los argumentos del binario.

	while ((token = strtok(NULL, " \n")) != NULL)
	{
		argv[argc] = new char[strlen(token) + 1];
		strncpy(argv[argc], token, strlen(token) + 1);
		argc++;
	}

	return argc;
}

//----------------------------------------------------------------------------------------
// Syscall_Halt().
//----------------------------------------------------------------------------------------

void Syscall_Halt()
{
	DEBUG('y', "[SYSCALL]: Shutdown, initiated by user program.\n");
	interrupt->Halt();
}

//----------------------------------------------------------------------------------------
// Syscall_Create().
//----------------------------------------------------------------------------------------

void Syscall_Create()
{
	DEBUG('y', "[SYSCALL]: Create file, initiated by user program.\n");

	// Obtenemos el <usrAddr> que contiene el nombre del archivo.

	int arg1 = machine->ReadRegister(4);

	// Obtenemos el nombre del archivo desde el espacio de usuario.

	int strLen = getStrLenFromUsr(arg1);
	char *fileName = new char[strLen + 1];
	readStrFromUsr(arg1, fileName);

	// Creamos el nuevo archivo.

	bool opRes = fileSystem->Create(fileName, NEW_FILE_SIZE);

	// Almacenamos en el registro el resultado de la operacion (0 OK, -1 BAD).

	if (opRes)
	{
		DEBUG('y', "[SYSCALL]: File %s created succesfully.\n", fileName);
		machine->WriteRegister(2, 0);
	}
	else
	{
		DEBUG('y', "[SYSCALL]: Failed to create file %s.\n", fileName);
		machine->WriteRegister(2, -1);
	}

	// Liberamos la memoria alocada.

	delete fileName;
}

//----------------------------------------------------------------------------------------
// Syscall_Read().
//----------------------------------------------------------------------------------------

void Syscall_Read()
{
	DEBUG('y', "[SYSCALL]: Read, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	int usrBuffAddr = machine->ReadRegister(4);
	int sizeToRead = machine->ReadRegister(5);
	OpenFileId fileDesc = machine->ReadRegister(6);

	// Creamos un buffer para almacenar los caracteres leidos.

	char *readBuff = new char[sizeToRead + 1];

	// Controlamos <fileDesc> para saber con que dispositivo estamos tratando.

	if (fileDesc == ConsoleOutput)
	{
		DEBUG('y', "[SYSCALL]: WARNING - Trying to read from STDOUT!\n");
		machine->WriteRegister(2, -1);
	}
	else if (fileDesc == ConsoleInput)
	{
		DEBUG('y', "[SYSCALL]: Reading from STDIN.\n");
		synchConsole->GetBuffer(readBuff, sizeToRead);
		writeBuffToUsr(readBuff, usrBuffAddr, sizeToRead);
		machine->WriteRegister(2, sizeToRead);
	}
	else
	{
		DEBUG('y', "[SYSCALL]: Reading from file, descriptor: %d.\n", fileDesc);

		OpenFile *file = fileDescTable->getFile(fileDesc);

		if (file == NULL)
		{
			DEBUG('y', "[SYSCALL]: ERROR - No file attached to descriptor.\n");
			machine->WriteRegister(2, -1);
		}
		else
		{
			int charsReaded = file->Read(readBuff, sizeToRead);
			readBuff[charsReaded] = '\0';
			DEBUG('y', "[SYSCALL]: Buffer readed: %s.\n", readBuff);
			writeBuffToUsr(readBuff, usrBuffAddr, charsReaded);
			machine->WriteRegister(2, charsReaded);
		}
	}

	// Liberamos la memoria alocada.

	delete readBuff;
}

//----------------------------------------------------------------------------------------
// Syscall_Write().
//----------------------------------------------------------------------------------------

void Syscall_Write()
{
	DEBUG('y', "[SYSCALL]: Write, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	int usrBuffAddr = machine->ReadRegister(4);
	int sizeToWrite = machine->ReadRegister(5);
	OpenFileId fileDesc = machine->ReadRegister(6);

	// Creamos un buffer para almacenar los caracteres a escribir en la consola.

	char *writeBuff = new char[sizeToWrite + 1];

	// Controlamos <fileDesc> para saber con que dispositivo estamos tratando.

	if (fileDesc == ConsoleInput)
	{
		DEBUG('y', "[SYSCALL]: WARNING - Trying to write to STDIN!\n");
		machine->WriteRegister(2, -1);
	}
	else if (fileDesc == ConsoleOutput)
	{
		DEBUG('y', "[SYSCALL]: Writing to STDOUT.\n");
		readBuffFromUsr(usrBuffAddr, writeBuff, sizeToWrite);
		synchConsole->PutBuffer(writeBuff, sizeToWrite);
		machine->WriteRegister(2, 0);
	}
	else
	{
		DEBUG('y', "[SYSCALL]: Writing to file, descriptor: %d.\n", fileDesc);

		OpenFile *file = fileDescTable->getFile(fileDesc);

		if (file == NULL)
		{
			DEBUG('y', "[SYSCALL]: ERROR - No file attached to descriptor.\n");
			machine->WriteRegister(2, -1);
		}
		else
		{
			readBuffFromUsr(usrBuffAddr, writeBuff, sizeToWrite);
			writeBuff[sizeToWrite] = '\0';
			DEBUG('y', "[SYSCALL]: Buffer to write: %s.\n", writeBuff);
			int charsWritten = file->Write(writeBuff, sizeToWrite);

			if (charsWritten < sizeToWrite)
				DEBUG('y', "[SYSCALL]: WARNING - Only %d chars written.\n", charsWritten);

			machine->WriteRegister(2, 0);
		}
	}

	// Liberamos la memoria alocada.

	delete writeBuff;
}

//----------------------------------------------------------------------------------------
// Syscall_Open().
//----------------------------------------------------------------------------------------

void Syscall_Open()
{
	DEBUG('y', "[SYSCALL]: Open, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	int usrFileNameAddr = machine->ReadRegister(4);

	// Obtenemos el nombre del archivo desde el espacio de usuario.

	int strLen = getStrLenFromUsr(usrFileNameAddr);
	char *fileName = new char[strLen + 1];
	readStrFromUsr(usrFileNameAddr, fileName);

	// Obtenemos un <OpenFile> a partir del nombre del archivo.

	OpenFile *file = fileSystem->Open(fileName);

	// Controlamos si existe algun archivo con dicho nombre.

	if (file == NULL)
	{
		DEBUG('y', "[SYSCALL]: ERROR - There is no such file!\n");
		machine->WriteRegister(2, -1);
	}
	else
	{
		OpenFileId id = fileDescTable->attachFile(file);
		DEBUG('y', "[SYSCALL]: File opened and attached to descriptor %d.\n", id);
		machine -> WriteRegister(2, id);
	}

	// Liberamos la memoria alocada para el nombre del archivo.

	delete fileName;
}

//----------------------------------------------------------------------------------------
// Syscall_Close().
//----------------------------------------------------------------------------------------

void Syscall_Close()
{
	DEBUG('y', "[SYSCALL]: Close, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	OpenFileId id = (OpenFileId) machine->ReadRegister(4);

	// Obtenemos el archivo a partir de su descriptor.

	OpenFile *file = fileDescTable->getFile(id);

	if (file == NULL)
	{
		DEBUG('y', "[SYSCALL]: ERROR - There is no file for descriptor %d!\n", id);
		machine->WriteRegister(2, -1);
	}
	else
	{
		fileDescTable->detachFile(id);
		delete file;
		DEBUG('y', "[SYSCALL]: File successfully closed for descriptor %d.\n", id);
		machine->WriteRegister(2, 0);
	}
}

//----------------------------------------------------------------------------------------
// Syscall_Exit().
//----------------------------------------------------------------------------------------

void Syscall_Exit()
{
	DEBUG('y', "[SYSCALL]: Exit, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	int exitValue = machine->ReadRegister(4);

	// Obtenemos el descriptor del thread.

	SpaceId id = processTable->getSpaceId(currentThread);

	// Sacamos el thread de la tabla de procesos.

	if (id >= 0)
		processTable->detachProcess(id, exitValue);

	// Finalizamos el thread.

	currentThread->Finish();
}

//----------------------------------------------------------------------------------------
// Syscall_Join().
//----------------------------------------------------------------------------------------

void Syscall_Join()
{
	DEBUG('y', "[SYSCALL]: Join, initiated by user program.\n");

	// Obtenemos los argumentos almacenados en los registros.

	SpaceId id = (SpaceId) machine->ReadRegister(4);

	// A partir del SpaceId obtenemos el thread asociado.

	Thread* threadToJoin = processTable->getThread(id);

	// Controlamos que el thread relacionado exista.

	if (threadToJoin == NULL)
	{
		DEBUG('y', "[SYSCALL]: JOIN ERROR: Thread to join is NULL!.\n");
		machine->WriteRegister(2, -1);
        return;
    }

	// Si el thread existe, realizamos un join sobre el (a nivel de thread).

	threadToJoin->Join();

	// Cuando <threadToJoin> finalice, obtenemos su valor de retorno.

	int exitValue;
	bool opRes = processTable->getExitValue(id, exitValue);

	if (opRes)
		machine->WriteRegister(2, exitValue);
	else
		machine->WriteRegister(2, -1);

	return;
}

//----------------------------------------------------------------------------------------
// Syscall_Exec().
//----------------------------------------------------------------------------------------

void Syscall_Exec()
{
	DEBUG('y', "[SYSCALL]: Exec, initiated by user program.\n");

	// Obtenemos el <usrAddr> que contiene el nombre del archivo.

	int fileNameAddr = machine->ReadRegister(4);

	// Obtenemos el nombre del archivo desde el espacio de usuario.

	int strLen = getStrLenFromUsr(fileNameAddr);
	char *fileName = new char[strLen + 1];
	readStrFromUsr(fileNameAddr, fileName);

	// Abrimos el archivo ejecutable, si hay falla retornamos -1.

	OpenFile *execFile = fileSystem->Open(fileName);

	if (execFile == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to open file %s!\n", fileName);
		delete fileName;
		machine->WriteRegister(2, -1);
		return;
	}

	DEBUG('y', "[SYSCALL]: Executable file %s successfully opened.\n", fileName);

	// Creamos el "AddrSpace" para el ejecutable y luego cerramos el archivo,
	// si hay falla retornamos -1.

	AddrSpace *addrSpace = new AddrSpace(execFile);
	delete execFile;

	if (addrSpace == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to create the address space!\n");
		delete fileName;
		machine->WriteRegister(2, -1);
		return;
	}

	// Creamos un nuevo thread y le asignamos el "AddrSpace".

	Thread *thread = new Thread(fileName, true);

	if (thread == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to create the thread!\n");
		delete fileName;
		delete addrSpace;
		machine->WriteRegister(2, -1);
		return;
	}

	thread->space = addrSpace;

	// Agregamos el thread nuevo a la tabla de procesos.

	SpaceId id = processTable->attachProcess(thread);

	if (id < 0)
	{
		DEBUG('y', "[SYSCALL]: Unable to attach the thread into process table!\n");
		delete fileName;
		delete addrSpace;
		delete thread;
		machine->WriteRegister(2, -1);
		return;
	}

	// Finalmente retornamos el SpaceId asignado y hacemos un fork() del thread.

	machine->WriteRegister(2, id);
	thread->Fork(RunProcess, NULL);
	return;
}

//----------------------------------------------------------------------------------------
// Syscall_ExecWithArgs().
//----------------------------------------------------------------------------------------

void Syscall_ExecWithArgs()
{
	DEBUG('y', "[SYSCALL]: Exec with arguments, initiated by user program.\n");

	// Obtenemos el <usrAddr> que contiene el nombre del archivo.

	int cmdStrAddr = machine->ReadRegister(4);

	// Obtenemos el string que describe el comando a ejecutar desde el
	// espacio de usuario.

	int strLen = getStrLenFromUsr(cmdStrAddr);
	char *cmdStr = new char[strLen + 1];
	readStrFromUsr(cmdStrAddr, cmdStr);

	// Obtenemos el path del ejecutable, y los argumentos.
	// XXX: Ojo, ParseCommand() aloca memoria internamente, hay que recordar eliminarla.

	char *filePath = NULL;
	char **argv = new char *[MAX_ARGS];
	int argc = ParseCommand(cmdStr, &filePath, argv);

	if (argc < 0)
	{
		DEBUG('y', "[SYSCALL]: Unable to parse the command %s!\n", cmdStr);
		delete cmdStr; delete argv;
		machine->WriteRegister(2, -1);
		return;
	}
	else
	{
		delete cmdStr;
	}

	// Abrimos el archivo ejecutable, si hay falla retornamos -1.

	OpenFile *execFile = fileSystem->Open(filePath);

	if (execFile == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to open file %s!\n", filePath);
		delete filePath;
		for (int i = 0; i < argc; i++) delete argv[i];
		delete argv;
		machine->WriteRegister(2, -1);
		return;
	}

	DEBUG('y', "[SYSCALL]: Executable file %s successfully opened.\n", filePath);

	// Creamos el "AddrSpace" para el ejecutable y luego cerramos el archivo,
	// si hay falla retornamos -1.

	AddrSpace *addrSpace = new AddrSpace(execFile);
	delete execFile;

	if (addrSpace == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to create the address space!\n");
		delete filePath;
		for (int i = 0; i < argc; i++) delete argv[i];
		delete argv;
		machine->WriteRegister(2, -1);
		return;
	}

	addrSpace->SetArguments(argc, argv);

	// Creamos un nuevo thread y le asignamos el "AddrSpace".

	Thread *thread = new Thread(filePath, true);

	if (thread == NULL)
	{
		DEBUG('y', "[SYSCALL]: Unable to create the thread!\n");
		delete filePath;
		for (int i = 0; i < argc; i++) delete argv[i];
		delete argv;
		delete addrSpace;
		machine->WriteRegister(2, -1);
		return;
	}

	thread->space = addrSpace;

	// Agregamos el thread nuevo a la tabla de procesos.

	SpaceId id = processTable->attachProcess(thread);

	if (id < 0)
	{
		DEBUG('y', "[SYSCALL]: Unable to attach the thread into process table!\n");
		delete filePath;
		for (int i = 0; i < argc; i++) delete argv[i];
		delete argv;
		delete addrSpace;
		delete thread;
		machine->WriteRegister(2, -1);
		return;
	}

	// Finalmente retornamos el SpaceId asignado y hacemos un fork() del thread.

	machine->WriteRegister(2, id);
	thread->Fork(RunProcess, NULL);
	return;
}

//----------------------------------------------------------------------------------------
// ExceptionHandler
// Entry point into the Nachos kernel. Called when a user program is executing, and
// either does a syscall, or generates an addressing or arithmetic exception.
//
// For system calls, the following is the calling convention:
//
// system call code -- r2
//             arg1 -- r4
//             arg2 -- r5
//             arg3 -- r6
//             arg4 -- r7
//
// The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. Or else you'll loop making the
// same system call forever!
//
// "which" is the kind of exception. The list of possible exceptions are in machine.h.
//----------------------------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	if (which == SyscallException)
	{
		switch (type)
		{
			case SC_Halt:
				Syscall_Halt();
				break;

			case SC_Create:
				Syscall_Create();
				IncreasePC();
				break;

			case SC_Read:
				Syscall_Read();
				IncreasePC();
				break;

			case SC_Write:
				Syscall_Write();
				IncreasePC();
				break;

			case SC_Open:
				Syscall_Open();
				IncreasePC();
				break;

			case SC_Close:
				Syscall_Close();
				IncreasePC();
				break;

			case SC_Exit:
				Syscall_Exit();
				IncreasePC();
				break;

			case SC_Join:
				Syscall_Join();
				IncreasePC();
				break;

			case SC_Exec:
				//Syscall_Exec();
				Syscall_ExecWithArgs();
				IncreasePC();
				break;

			default:
				printf("[SYSCALL]: Unexpected user mode exception %d %d\n", which, type);
				ASSERT(false);
				break;
		}
	}
	else if (which == PageFaultException)
	{
#ifdef USE_TLB

		int missVAddr = machine->ReadRegister(BadVAddrReg);
		int missVPage = missVAddr / PageSize;

		DEBUG('v', "[TLB]: Missing virtual address %d, from virtual page %d\n",
              missVAddr, missVPage);

		tlbHandler->UpdateTLB(missVPage);

#endif
	}
	else if (which == ReadOnlyException)
	{
#ifdef USE_TLB

		ASSERT(false);

#endif
	}
	else
	{
		printf("[SYSCALL]: Unexpected user mode exception %d %d\n", which, type);
		ASSERT(false);
	}
}
