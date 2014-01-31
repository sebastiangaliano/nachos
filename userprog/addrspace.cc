// addrspace.cc
// Routines to manage address spaces (executing user programs).
//
// In order to run a user program, you must:
//
// 1. link with the -N -T 0 option
// 2. run coff2noff to convert the object file to Nachos format (Nachos object code
//    format is essentially just a simpler version of the UNIX executable object
//    code format)
// 3. load the NOFF file into the Nachos file system (if you haven't implemented the
//    file system yet, you don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "system.h"
#include "addrspace.h"


//----------------------------------------------------------------------------------------
// SwapHeader
// Do little endian to big endian conversion on the bytes in the object file header, in
// case the file was generated on a little endian machine, and we're now running on a
// big endian machine.
//----------------------------------------------------------------------------------------

static void SwapHeader(NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------------------------
// AddrSpace::TranslateMem
// Translate a virtual address into a physical address and calculates offset.
//----------------------------------------------------------------------------------------

void AddrSpace::TranslateMem(int virtualAddr, int* physicalAddr, int* offset)
{
	// Assert that pageTable exists.

	ASSERT(pageTable != NULL);

	// Calculates physical address and offset.

	int numOfPage = virtualAddr / PageSize;
	int physicalPageNum = pageTable[numOfPage].physicalPage;
	*offset = virtualAddr % PageSize;
	*physicalAddr = (physicalPageNum * PageSize) + *offset;

	DEBUG('a', "Translated VA: %d to PA: %d [VP: %d PP: %d OFFSET: %d]\n",
          virtualAddr, *physicalAddr, numOfPage, physicalPageNum, *offset);
}

//----------------------------------------------------------------------------------------
// AddrSpace::CopySegment
// Copy a segment of a user program into the physical address.
//----------------------------------------------------------------------------------------

void AddrSpace::CopySegment(Segment seg, OpenFile *executable)
{
	int segSize, offset, sizeToCopy, spaceLeftOnPage, physicalAddr;

	// Check the segment size.

	if (seg.size <= 0)
		return;

	// Copy segment into physical address.

	DEBUG('a', "Initializing segment at VA %d, size %d\n", seg.virtualAddr, seg.size);

	segSize = seg.size;

	while (segSize > 0)
	{
		sizeToCopy = segSize < PageSize ? segSize : PageSize;

		TranslateMem(seg.virtualAddr, &physicalAddr, &offset);

		if (offset != 0)
		{
			spaceLeftOnPage = PageSize - offset;
			sizeToCopy = sizeToCopy < spaceLeftOnPage ? sizeToCopy : spaceLeftOnPage;
		}

		DEBUG('a', "Copying %d bytes of segment into physical address\n", sizeToCopy);

		executable->ReadAt(&(machine->mainMemory[physicalAddr]),
                           sizeToCopy, seg.inFileAddr);

		seg.virtualAddr += sizeToCopy;
		seg.inFileAddr += sizeToCopy;
		segSize -= sizeToCopy;
	}
}

//----------------------------------------------------------------------------------------
// AddrSpace::PushArgsOnStack
// Push arguments on stack and moves stack pointer. Returns the new stack pointer.
//----------------------------------------------------------------------------------------

int AddrSpace::PushArgsOnStack(int argc, char **argv)
{
	int stack_ptr, arg_len;
	int argv_ptr[argc];
	char *arg;

	// Obtenemos el puntero al final del stack.

	stack_ptr = numPages * PageSize;

	// Agregamos los argumentos de <argv> en el stack y recalculamos el <stack_ptr>.
	// En argv_ptr[i] almacenamos la direccion en el stack del argumento i-esimo.

	for (int i = 0; i < argc; i++)
	{
		arg = argv[i];
		arg_len = strlen(arg) + 1;
		stack_ptr -= arg_len;

		for (int k = 0; k < arg_len; k++)
			machine->WriteMem(stack_ptr + k, 1, arg[k]);

		argv_ptr[i] = stack_ptr;
	}

	// Movemos el stack pointer para reservar el espacio donde escribiremos los punteros
	// a los argumentos y el puntero a NULL. XXX: Este ultimo stack pointer debe quedar
	// alineado a una direccion multiplo de 4.

	stack_ptr -= 4 * (argc + 1);
	stack_ptr -= (stack_ptr % 4);

	// Agregamos los punteros a los argumentos en el stack.

	for (int i = 0; i < argc; i++)
		machine->WriteMem(stack_ptr + (4 * i), 4, argv_ptr[i]);

	// Agregamos el puntero a NULL en el stack.

	machine->WriteMem(stack_ptr + (4 * argc), 4, 0);

	// Retornamos el nuevo stack pointer.

	return stack_ptr;
}

//----------------------------------------------------------------------------------------
// AddrSpace::AddrSpace
// Create an address space to run a user program. Load the program from a file
// "executable", and set everything up so that we can start executing user instructions.
//
// Assumes that the object code file is in NOFF format.
//
// First, set up the translation from program memory to physical memory. For now, this
// is really simple (1:1), since we are only uniprogramming, and we have a single
// unsegmented page table.
//
// "executable" is the file containing the object code to load into memory.
//----------------------------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
	NoffHeader noffH;
	unsigned int i, size, pageOffset;
	int freeMemPageNum;

	// Read the executable header.

	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

	if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);

	ASSERT(noffH.noffMagic == NOFFMAGIC);

	// How big is address space?
	// We need to increase the size to leave room for the stack.

	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

	// Check we're not trying to run anything too big -- at least until we have
	// virtual memory.

	ASSERT(numPages <= (unsigned int)memoryBitMap->NumClear());

	DEBUG('a', "-----------------------------------------------------------\n");
	DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
	DEBUG('a', "-----------------------------------------------------------\n");

	// First, set up the translation.

	pageTable = new TranslationEntry[numPages];

	for (i = 0; i < numPages; i++) {

		// Search for a free page in memory.

		freeMemPageNum = memoryBitMap->Find();

		if (freeMemPageNum < 0)
		{
			DEBUG('a', "ERROR - Could not allocate page number %d\n", i);
			break;
		}

		DEBUG('a', "Assigning physical page %d to virtual page %d\n", freeMemPageNum, i);

		// Setup the i-element of pageTable.

		pageTable[i].virtualPage = i;
		pageTable[i].physicalPage = freeMemPageNum;
		pageTable[i].valid = true;
		pageTable[i].use = false;
		pageTable[i].dirty = false;

		// If the code segment was entirely on a separate page, we could set its
		// pages to be read-only.

		pageTable[i].readOnly = false;

		// Zero out the related memory page, to zero the unitialized data segment
		// and the stack segment.

		pageOffset = freeMemPageNum * PageSize;
		bzero(&machine->mainMemory[pageOffset], PageSize);
		DEBUG('a', "Zero out memory from byte %d to byte %d.\n",
              pageOffset, pageOffset + PageSize);
	}

	// Then, copy the code and data segment into memory.

	DEBUG('a', "-----------------------------------------------------------\n");
	DEBUG('a', "Copying the CODE segment...\n");
	DEBUG('a', "-----------------------------------------------------------\n");
	CopySegment(noffH.code, executable);
	DEBUG('a', "-----------------------------------------------------------\n");
	DEBUG('a', "Copying the DATA segment...\n");
	DEBUG('a', "-----------------------------------------------------------\n");
	CopySegment(noffH.initData, executable);

	// Seteamos los campos asociados a Exec().

	has_arguments = false;
	argc_real = 0;
	argv_real = NULL;
}

//----------------------------------------------------------------------------------------
// AddrSpace::~AddrSpace
// Deallocate an address space.
//----------------------------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	for (int i = 0; i < argc_real; i++) {
		DEBUG('x', "Deleting arg %d: %s!\n", i, argv_real[i]);
		delete argv_real[i];
	}

	delete argv_real;

	for (unsigned int i = 0; i < numPages; i++)
		memoryBitMap->Clear(pageTable[i].physicalPage);

	delete pageTable;
}

//----------------------------------------------------------------------------------------
// AddrSpace::InitRegisters
// Set the initial values for the user-level register set.
//
// We write these directly into the "machine" registers, so that we can immediately jump
// to user code. Note that these will be saved/restored into the
// currentThread->userRegisters when this thread is context switched out.
//----------------------------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);

	// Initial program counter -- must be location of "Start".

	machine->WriteRegister(PCReg, 0);

	// Need to also tell MIPS where next instruction is, because of branch
	// delay possibility.

	machine->WriteRegister(NextPCReg, 4);

	// If this address space has arguments, we push they into the stack.

	int sp = numPages * PageSize;

	if (has_arguments) {
		sp = PushArgsOnStack(argc_real, argv_real);
		machine->WriteRegister(4, argc_real);
		machine->WriteRegister(5, sp);
	}

	// Set the stack register to the end of the address space, where we allocated the
	// stack; but subtract off a bit, to make sure we don't accidentally reference off
	// the end!

	machine->WriteRegister(StackReg, sp - 16);
	DEBUG('a', "Initializing stack register to %d\n", sp - 16);
}

//----------------------------------------------------------------------------------------
// AddrSpace::SaveState
// On a context switch, save any machine state, specific to this address space,
// that needs saving.
//----------------------------------------------------------------------------------------

void AddrSpace::SaveState()
{
#ifdef USE_TLB

	for (int i = 0; i < TLBSize; i++)
		if (machine->tlb[i].valid && machine->tlb[i].dirty)
			pageTable[machine->tlb[i].virtualPage] = machine->tlb[i];

#else

	pageTable = machine->pageTable;
	numPages = machine->pageTableSize;

#endif
}

//----------------------------------------------------------------------------------------
// AddrSpace::RestoreState
// On a context switch, restore the machine state so that this address space can run.
//
// For now, tell the machine where to find the page table.
//----------------------------------------------------------------------------------------

void AddrSpace::RestoreState()
{
#ifdef USE_TLB

	for (int i = 0; i < TLBSize; i++)
		machine->tlb[i].valid = false;

#else

	machine->pageTable = pageTable;
	machine->pageTableSize = numPages;

#endif
}

//----------------------------------------------------------------------------------------
// AddrSpace::SetArguments
// Set argc and argv values for this address space.
//----------------------------------------------------------------------------------------

void AddrSpace::SetArguments(int argc, char **argv)
{
	// Debuggeamos los argumentos recibidos, para control.

	DEBUG('y', "-----------------------------------------------------------\n");
	DEBUG('y', "[ADDRSPACE]: Setting arguments:\n");

	for (int i = 0; i < argc; i++)
		DEBUG('y', "[ADDRSPACE]: ARG %d: %s\n", i, argv[i]);

	DEBUG('y', "-----------------------------------------------------------\n");

	// Almacenamos los argumentos 

	has_arguments = true;
	argc_real = argc;
	argv_real = argv;
}

//----------------------------------------------------------------------------------------
// AddrSpace::GetPage
// Obtain an entry from the corresponding page table.
//----------------------------------------------------------------------------------------

TranslationEntry* AddrSpace::GetPage(int numPage)
{
	return &pageTable[numPage];
}
