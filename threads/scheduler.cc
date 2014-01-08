// scheduler.cc
// Routines to choose the next thread to run, and to dispatch that thread.
//
// These routines assume that interrupts are already disabled. If interrupts are disabled,
// we can assume mutual exclusion (since we are on a uniprocessor).
//
// NOTE: We can't use Locks to provide mutual exclusion here, since if we needed to wait
// for a lock, and the lock was busy, we would end up calling FindNextToRun(), and that
// would put us in an infinite loop.
//
// Very simple implementation -- no priorities, straight FIFO. Might need to be improved
// in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "scheduler.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// Scheduler::Scheduler
// Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------------------------

Scheduler::Scheduler()
{
	//readyList = new List<Thread*>;

	// Inicializamos las colas de prioridades.

	for (int p = 0; p <= _MAX_PRIORITY; p++)
		readyList[p] = new List<Thread*>;
}

//----------------------------------------------------------------------------------------
// Scheduler::~Scheduler
// De-allocate the list of ready threads.
//----------------------------------------------------------------------------------------

Scheduler::~Scheduler()
{
 	//delete readyList;

	// Eliminamos las colas creadas con new.

	for (int p = 0; p <= _MAX_PRIORITY; p++) {
		delete readyList[p];
		readyList[p] = NULL;
	}
}

//----------------------------------------------------------------------------------------
// Scheduler::ReadyToRun
// Mark a thread as ready, but not running. Put it on the ready list, for later
// scheduling onto the CPU.
//
// "thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread)
{
	//DEBUG('t', "Putting thread %s on ready list.\n", thread->getName());

	//thread->setStatus(READY);
	//readyList->Append(thread);

	int p = thread->getPriority();
	DEBUG('t', "Putting thread %s on ready list %d.\n", thread->getName(), p);

	thread->setStatus(READY);
	readyList[p]->Append(thread);
}

//----------------------------------------------------------------------------------------
// Scheduler::FindNextToRun
// Return the next thread to be scheduled onto the CPU. If there are no ready threads,
// return NULL.
//
// Side effect: Thread is removed from the ready list.
//----------------------------------------------------------------------------------------

Thread* Scheduler::FindNextToRun()
{
	//return readyList->Remove();

	// Buscamos el primer thread de la cola con mas prioridad.

	for (int p = _MAX_PRIORITY; p >= 0; p--)
		if (!(readyList[p]->IsEmpty()))
			return readyList[p]->Remove();

	// Si no hay threads listos, retornamos NULL.

	return NULL;
}

//----------------------------------------------------------------------------------------
// Scheduler::Run
// Dispatch the CPU to nextThread. Save the state of the old thread, and load the state
// of the new thread, by calling the machine dependent context switch routine, SWITCH.
//
// Note: we assume the state of the previously running thread has already been changed
// from running to blocked or ready (depending).
// Side effect:
// The global variable currentThread becomes nextThread.
//
// "nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread)
{
	Thread *oldThread = currentThread;

#ifdef USER_PROGRAM		// Ignore until running user programs.

	// If this thread is a user program, save the user's CPU registers.

	if (currentThread->space != NULL) {
		currentThread->SaveUserState();
		currentThread->space->SaveState();
    }

#endif

	// Check if the old thread had an undetected stack overflow.

	oldThread->CheckOverflow();

	// Switch to the next thread, nextThread is now running.

	currentThread = nextThread;
	currentThread->setStatus(RUNNING);

    DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n",
          oldThread->getName(), nextThread->getName());

	// This is a machine-dependent assembly language routine defined in switch.s. You may
	// have to think a bit to figure out what happens after this, both from the point
	// of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);
    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

	// If the old thread gave up the processor because it was finishing, we need to
	// delete its carcass.  Note we cannot delete the thread before now (for example, in
	// Thread::Finish()), because up to this point, we were still running on the old
	// thread's stack!.

	if (threadToBeDestroyed != NULL) {
		delete threadToBeDestroyed;
		threadToBeDestroyed = NULL;
    }

#ifdef USER_PROGRAM

	// If there is an address space to restore, do it.

	if (currentThread->space != NULL) {
		currentThread->RestoreUserState();
		currentThread->space->RestoreState();
    }

#endif

}

//----------------------------------------------------------------------------------------
// Scheduler::RemoveFromList
// Sacar el primer elemento de una de las colas de prioridades.
//
// "prior" es la prioridad asociada a la cola de la cual se desea sacar el elemento.
//----------------------------------------------------------------------------------------

Thread* Scheduler::RemoveFromList(int prior)
{
	return readyList[prior]->Remove();
}

//----------------------------------------------------------------------------------------
// Scheduler::Print
// Print the scheduler state -- in other words, the contents of the ready list. For
// debugging.
//----------------------------------------------------------------------------------------

static void ThreadPrint(Thread* t) {
	t->Print();
}

void Scheduler::Print()
{
	for (int p = 0; p <= _MAX_PRIORITY; p++)
	{
		printf("-----------------------------\n");
		printf("Ready list [%d] contents:\n", p);
		printf("-----------------------------\n");
		readyList[p]->Apply(ThreadPrint);
		printf("\n");
	}
}
