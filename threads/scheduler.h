// scheduler.h
// Data structures for the thread dispatcher and scheduler. Primarily, the list of
// threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved. See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"


//----------------------------------------------------------------------------------------
// The following class defines the scheduler/dispatcher abstraction -- the data
// structures and operations needed to keep track of which thread is running, and which
// threads are ready but not running.
//----------------------------------------------------------------------------------------

class Scheduler {

public:

	// Initialize list of ready threads.

	Scheduler();

	// De-allocate ready list.

	~Scheduler();

	// Thread can be dispatched.

	void ReadyToRun(Thread* thread);

	// Dequeue first thread on the ready list, if any, and return thread.

	Thread* FindNextToRun();

	// Cause nextThread to start running.

	void Run(Thread* nextThread);

	// Print contents of ready list.

	void Print();

	// Saca el primer elemento de una de las colas de prioridades.

	Thread* RemoveFromList(int prior);

private:

	// Queue of threads that are ready, but not running (old).

	//List<Thread*>* readyList;

	// Arreglo de colas de threads (cada cola esta asignada a una prioridad).

	List<Thread*>* readyList[_MAX_PRIORITY + 1];

};

#endif // SCHEDULER_H
