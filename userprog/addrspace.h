// addrspace.h
// Data structures to keep track of executing user programs (address spaces).
//
// For now, we don't keep any information about address spaces. The user level CPU state
// is saved and restored in the thread executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"

#define UserStackSize		1024 	// Increase this as necessary!


class AddrSpace {

public:

	// Create an address space, initializing it with the program stored in the
	// file "executable".

	AddrSpace(OpenFile *executable);

	// De-allocate an address space.

	~AddrSpace();

	// Initialize user-level CPU registers, before jumping to user code.

	void InitRegisters();

	// Save/restore address space-specific info on a context switch.

	void SaveState();
	void RestoreState();

	// Sets arguments on this address space (argc/argv).

	void SetArguments(int argc, char **argv);

	// Get the arguments array of this address space.

	char** GetArguments() { return argv_real; }

private:

	TranslationEntry *pageTable;	// Assume linear page table translation for now!
	unsigned int numPages;			// Number of pages in the virtual address space.

	// Private methods.

	void TranslateMem(int virtualAddr, int* physicalAddr, int* offset);
	void CopySegment(Segment seg, OpenFile *executable);

	// Push the arguments of this address space on the stack. Return the new stack
	// pointer.

	int PushArgsOnStack(int argc, char **argv);

	// Fields for support Exec with arguments.

	bool has_arguments;				// True if this address space has arguments.
	int argc_real;					// Number of arguments.
	char **argv_real;				// Array of arguments.
};


#endif // ADDRSPACE_H
