// thread.h
// Data structures for managing threads. A thread represents sequential execution of code
// within a program. So the state of a thread includes the program counter, the processor
// registers, and the execution stack.
//
// Note that because we allocate a fixed size stack for each thread, it is possible to
// overflow the stack -- for instance, by recursing to too deep a level. The most common
// reason for this occuring is allocating large data structures on the stack. For
// instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
// Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
// Bad things happen if you overflow the stack, and in the worst case, the problem may
// not be caught explicitly.  Instead, the only symptom may be bizarre segmentation
// faults. (Of course, other problems can cause seg faults, so that isn't a sure sign
// that your thread stacks are too small.)
//
// One thing to try if you find yourself with seg faults is to increase the size of
// thread stack -- ThreadStackSize.
//
// In this interface, forking a thread takes two steps. We must first allocate a data
// structure for it: "t = new Thread". Only then can we do the fork: "t->Fork(f, arg)".
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef THREAD_H
#define THREAD_H

#include "copyright.h"
#include "utility.h"

#ifdef USER_PROGRAM
#include "machine.h"
#include "addrspace.h"
#endif


// CPU register state to be saved on context switch.
// x86 processors needs 9 32-bit registers, whereas x64 has 8 extra registers.
// We allocate room for the maximum of these two architectures

const int MachineStateSize = 17;

// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!

const int StackSize = 4 * 1024; // In words.

// Thread state.

enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED };

// Definimos el numero maximo de prioridades para los threads.

#define _MAX_PRIORITY 5


//----------------------------------------------------------------------------------------
// The following class defines a "thread control block" -- which represents a single
// thread of execution.
//
// Every thread has:
//	- An execution stack for activation records ("stackTop" and "stack").
//	- Space to save CPU registers while not running ("machineState").
//	- A "status" (running/ready/blocked).
//
// Some threads also belong to a user address space; threads that only run in the kernel
// have a NULL address space.
//----------------------------------------------------------------------------------------

// Declaramos los tipos de datos que necesitan ser reconocidos por el modulo. Estas
// declaraciones son necesarias para la implementacion del metodo JOIN. TODO: Investigar
// si conviene utilizar un puerto para la implementacion del JOIN.

class Semaphore;
class Lock;
class Condition;
//class Port;


class Thread {

private:

	// NOTE: DO NOT CHANGE the order of these first two members.
	// THEY MUST be in this position for SWITCH to work.

	HostMemoryAddress* stackTop;						// The current stack pointer.
	HostMemoryAddress machineState[MachineStateSize];	// All registers except for stackTop.

public:

	// Initialize a Thread.

	Thread(const char* debugName, bool joinable = false, int p = 0);

	// Deallocate a Thread.
	// NOTE: thread being deleted must not be running when delete is called.

	~Thread();

	// Make thread run (*func)(arg).

	void Fork(VoidFunctionPtr func, void* arg);

	// Relinquish the CPU if any other thread is runnable.

	void Yield();

	// Put the thread to sleep and relinquish the processor.

	void Sleep();

	// The thread is done executing.

	void Finish();

	// Check if thread has overflowed its stack.

	void CheckOverflow();

	// Other operations.

	void setStatus(ThreadStatus st) { status = st; }
	ThreadStatus getStatus() { return status; }
	const char* getName() { return (name); }
	void Print() { printf("%s, ", name); }

	// Join Method.

	void Join();

	// Priority Methods.

	int getPriority() { return priority; }
	int setPriority(int p);
	int getInitialPriority() { return init_priority; }

private:

	// Some of the private data for this class is listed above.

	HostMemoryAddress* stack;	// Bottom of the stack. NULL if this is the main thread.
								// (If NULL, don't deallocate stack).
	ThreadStatus status;		// Status: ready, running or blocked.
	const char* name;			// Name of the thread.

	// Datos privados asociados al uso de JOIN.

	bool isJoinable;            // Indica si podemos realizar Join sobre el thread.
	Condition* joinCV;          // Variable de condicion para implementacion del Join.
	Lock* joinLock;             // Lock para la CV definida anteriormente.
	Semaphore* joinSem;         // Semaforo para controlar la invocacion a Join.
	//Port* joinPort;             // Puerto para implementacion alternativa del Join.

	char* joinCVName;           // Nombre de la CV para implementacion del Join.
	char* joinLockName;         // Nombre del lock para la CV definida anteriormente.
	char* joinSemName;          // Nombre del semaforo para controlar la invocacion a Join.

	// Datos privados asociados al uso de prioridad.

	int priority;				// Prioridad del thread (modificable).
	int init_priority;			// Prioridad del thread (original, no modificable).

	// Allocate a stack for thread. Used internally by Fork().

	void StackAllocate(VoidFunctionPtr func, void* arg);

#ifdef USER_PROGRAM

	// A thread running a user program actually has *two* sets of CPU registers -- one
	// for its state while executing user code, one for its state while executing
	// kernel code.

	int userRegisters[NumTotalRegs];	// User-level CPU register state.

public:

	void SaveUserState();		// Save user-level register state.
	void RestoreUserState();	// Restore user-level register state.

	AddrSpace *space;			// User code this thread is running.

#endif
};

// Magical machine-dependent routines, defined in <switch.s>.

extern "C" {

// First frame on thread execution stack;
// enable interrupts.
// call "func".
// (when func returns, if ever) call ThreadFinish().

void ThreadRoot();

// Stop running oldThread and start running newThread.

void SWITCH(Thread *oldThread, Thread *newThread);

}

#endif // THREAD_H
