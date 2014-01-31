// system.h
// All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved. See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"


//----------------------------------------------------------------------------------------
// Initialization and cleanup routines.
//----------------------------------------------------------------------------------------

// Initialization, called before anything else.

extern void Initialize(int argc, char **argv);

// Cleanup, called when Nachos is done.

extern void Cleanup();

//----------------------------------------------------------------------------------------
// Global variables.
//----------------------------------------------------------------------------------------

extern Thread *currentThread;			// The thread holding the CPU.
extern Thread *threadToBeDestroyed;		// The thread that just finished.
extern Scheduler *scheduler;			// The ready list.
extern Interrupt *interrupt;			// Interrupt status.
extern Statistics *stats;				// Performance metrics.
extern Timer *timer;					// The hardware alarm clock.

#ifdef USER_PROGRAM
#include "machine.h"
#include "synchconsole.h"
#include "fdtable.h"
#include "processtable.h"
#include "bitmap.h"
extern Machine* machine;				// User program memory and registers.
extern SynchConsole *synchConsole;		// For synchronize access to console.
extern FDTable *fileDescTable;			// File Descriptor Table.
extern ProcessTable *processTable;		// System Process Table.
extern BitMap *memoryBitMap;			// A BitMap related to address spaces.
#endif

#ifdef FILESYS_NEEDED					// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#ifdef VM
#include "tlbhandler.h"

extern TlbHandler* tlbHandler;
#endif

#endif // SYSTEM_H
