// thread.cc
// Routines to manage threads.  There are four main operations:
//
// Fork   -- Create a thread to run a procedure concurrently with the caller (this is done
//           in two steps -- first allocate the Thread object, then call Fork on it).
// Finish -- Called when the forked procedure finishes, to clean up.
// Yield  -- Relinquish control over the CPU to another ready thread.
// Sleep  -- Relinquish control over the CPU, but thread is now blocked. In other words,
//           it will not run again, until explicitly put back on the ready queue.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "system.h"
//#include "port.h"


// This is put at the top of the execution stack, for detecting stack overflows.

const unsigned STACK_FENCEPOST = 0xdeadbeef;

//----------------------------------------------------------------------------------------
// Thread::Thread -- Updated constructor.
// Initialize a thread control block, so that we can then call Thread::Fork.
//
// "threadName" is an arbitrary string, useful for debugging.
// "joinable" indica si el thread estara habilitado para operaciones Join().
// "p" indica la prioridad del thread, 0 por defecto.
//----------------------------------------------------------------------------------------

Thread::Thread(const char* threadName, bool joinable, int p)
{
	name = threadName;
	stackTop = NULL;
	stack = NULL;
	status = JUST_CREATED;

#ifdef USER_PROGRAM
	space = NULL;
#endif

	// Inicializamos datos exclusivos para implementacion del Join().

	isJoinable = joinable;

	if (isJoinable) {

		int nameSize = strlen(name);

		joinLockName = new char[nameSize + 10];
		strcpy(joinLockName, name);
		strcat(joinLockName, ".Lock");
		joinLock = new Lock(joinLockName);

		joinCVName = new char[nameSize + 10];
		strcpy(joinCVName, name);
		strcat(joinCVName, ".Cond");
		joinCV = new Condition(joinCVName, joinLock);

		joinSemName = new char[nameSize + 10];
		strcpy(joinSemName, name);
		strcat(joinSemName, ".Sem");
		joinSem = new Semaphore(joinSemName, 0);

		//char* auxName = new char[nameSize + 10];
		//strcpy(auxName, name);
		//strcat(auxName, ".Port");
		//joinPort = new Port(auxName);
	}

	// Inicializamos la prioridad del thread (0 por defecto, la menor).

	init_priority = setPriority(p);
}

//----------------------------------------------------------------------------------------
// Thread::~Thread
// De-allocate a thread.
//
// NOTE: the current thread *cannot* delete itself directly, since it is still running on
// the stack that we need to delete.
//
// NOTE: if this is the main thread, we can't delete the stack because we didn't allocate
// it -- we got it automatically as part of starting up Nachos.
//----------------------------------------------------------------------------------------

Thread::~Thread()
{
	DEBUG('t', "Deleting thread \"%s\"\n", name);

	ASSERT(this != currentThread);

#ifdef USER_PROGRAM
	DEBUG('x', "Deleting name and space of thread \"%s\"\n", name);
	delete name;
	delete space;
#endif

	if (stack != NULL)
		DeallocBoundedArray((char *) stack, StackSize * sizeof(HostMemoryAddress));
}

//----------------------------------------------------------------------------------------
// Thread::Fork
// Invoke (*func)(arg), allowing caller and callee to execute concurrently.
//
// NOTE: although our definition allows only a single integer argument to be passed to
// the procedure, it is possible to pass multiple arguments by making them fields of a
// structure, and passing a pointer to the structure as "arg".
//
// Implemented as the following steps:
//    1. Allocate a stack.
//	  2. Initialize the stack so that a call to SWITCH will cause it to run the procedure.
//	  3. Put the thread on the ready queue.
//
// "func" is the procedure to run concurrently.
// "arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------------------------

void Thread::Fork(VoidFunctionPtr func, void* arg)
{
#ifdef HOST_x86_64
	DEBUG('t', "Forking thread \"%s\" with func = 0x%lx, arg = %ld\n",
          name, (HostMemoryAddress) func, arg);
#else
	// Code for 32-bit architectures.
	DEBUG('t', "Forking thread \"%s\" with func = 0x%x, arg = %d\n",
          name, (HostMemoryAddress) func, arg);
#endif

	StackAllocate(func, arg);

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	scheduler->ReadyToRun(this);	// ReadyToRun assumes that interrupts are disabled!
	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// Thread::CheckOverflow
// Check a thread's stack to see if it has overrun the space that has been allocated for
// it. If we had a smarter compiler, we wouldn't need to worry about this, but we don't.
//
// NOTE: Nachos will not catch all stack overflow conditions. In other words, your
// program may still crash because of an overflow.
//
// If you get bizarre results (such as seg faults where there is no code) then you *may*
// need to increase the stack size. You can avoid stack overflows by not putting large
// data structures on the stack.
// Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------------------------

void Thread::CheckOverflow()
{
	if (stack != NULL) {
		ASSERT(*stack == STACK_FENCEPOST);
	}
}

//----------------------------------------------------------------------------------------
// Thread::Finish
// Called by ThreadRoot when a thread is done executing the forked procedure.
//
// NOTE: we don't immediately de-allocate the thread data structure or the execution
// stack, because we're still running in the thread and we're still on the stack! Instead,
// we set "threadToBeDestroyed", so that Scheduler::Run() will call the destructor,
// once we're running in the context of a different thread.
//
// NOTE: we disable interrupts, so that we don't get a time slice between setting
// threadToBeDestroyed, and going to sleep.
//----------------------------------------------------------------------------------------

void Thread::Finish()
{
	interrupt->SetLevel(IntOff);
	ASSERT(this == currentThread);

	DEBUG('t', "Finishing thread \"%s\"\n", getName());

	if (isJoinable) {

		// Si el JOIN no fue invocado, no se podra adquirir el semaforo y
		// quedaremos bloqueados.

		DEBUG('j', "[THREAD-JOIN]: Thread %s waiting for join()...\n", getName());
		joinSem->P();

		// Hacemos un broadcast sobre la variable de condicion asociada.

		joinLock->Acquire();
		joinCV->Broadcast();
		joinLock->Release();

		// Enviamos una mensaje por el puerto, se sincroniza con el thread receptor.
		//joinPort->Send(1);
	}

	DEBUG('j', "[THREAD-JOIN]: Thread %s finished!\n", getName());

	threadToBeDestroyed = currentThread;
	Sleep();	// Invokes SWITCH.
	// Not reached.
}

//----------------------------------------------------------------------------------------
// Thread::Yield
// Relinquish the CPU if any other thread is ready to run. If so, put the thread on the
// end of the ready list, so that it will eventually be re-scheduled.
//
// NOTE: returns immediately if no other thread on the ready queue. Otherwise returns
// when the thread eventually works its way to the front of the ready list and gets
// re-scheduled.
//
// NOTE: we disable interrupts, so that looking at the thread on the front of the ready
// list, and switching to it, can be done atomically. On return, we re-set the interrupt
// level to its original state, in case we are called with interrupts disabled.
//
// Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------------------------

void Thread::Yield ()
{
	Thread *nextThread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	ASSERT(this == currentThread);

	DEBUG('t', "Yielding thread \"%s\"\n", getName());

	nextThread = scheduler->FindNextToRun();

	if (nextThread != NULL) {
		scheduler->ReadyToRun(this);
		scheduler->Run(nextThread);
	}

	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// Thread::Sleep
// Relinquish the CPU, because the current thread is blocked waiting on a synchronization
// variable (Semaphore, Lock, or Condition). Eventually, some thread will wake this
// thread up, and put it back on the ready queue, so that it can be re-scheduled.
//
// NOTE: if there are no threads on the ready queue, that means we have no thread to run.
// "Interrupt::Idle" is called to signify that we should idle the CPU until the next I/O
// interrupt occurs (the only thing that could cause a thread to become ready to run).
//
// NOTE: we assume interrupts are already disabled, because it is called from the
// synchronization routines which must disable interrupts for atomicity. We need
// interrupts off so that there can't be a time slice between pulling the first thread
// off the ready list, and switching to it.
//----------------------------------------------------------------------------------------

void Thread::Sleep ()
{
	Thread *nextThread;
    
	ASSERT(this == currentThread);
	ASSERT(interrupt->getLevel() == IntOff);
    
	DEBUG('t', "Sleeping thread \"%s\"\n", getName());

	status = BLOCKED;

	while ((nextThread = scheduler->FindNextToRun()) == NULL) {
		interrupt->Idle();	// No one to run, wait for an interrupt.
	}

	scheduler->Run(nextThread); // Returns when we've been signalled
}

//----------------------------------------------------------------------------------------
// ThreadFinish, InterruptEnable
// Dummy functions because C++ does not allow a pointer to a member function.
// So in order to do this, we create a dummy C function (which we can pass a pointer to),
// that then simply calls the member function.
//----------------------------------------------------------------------------------------

static void ThreadFinish() { currentThread->Finish(); }
static void InterruptEnable() { interrupt->Enable(); }

//----------------------------------------------------------------------------------------
// Thread::StackAllocate
// Allocate and initialize an execution stack. The stack is initialized with an initial
// stack frame for ThreadRoot, which:
//		Enables interrupts.
//		Calls (*func)(arg).
//		Calls Thread::Finish.
//
// "func" is the procedure to be forked.
// "arg" is the parameter to be passed to the procedure.
//----------------------------------------------------------------------------------------

void Thread::StackAllocate (VoidFunctionPtr func, void* arg)
{
	stack = (HostMemoryAddress *) AllocBoundedArray(StackSize * sizeof(HostMemoryAddress));

	// i386 & MIPS & SPARC stack works from high addresses to low addresses
	stackTop = stack + StackSize - 4;	// -4 to be on the safe side!

	// the 80386 passes the return address on the stack.  In order for
	// SWITCH() to go to ThreadRoot when we switch to this thread, the
	// return addres used in SWITCH() must be the starting address of
	// ThreadRoot.
	*(--stackTop) = (HostMemoryAddress)ThreadRoot;

	*stack = STACK_FENCEPOST;

	machineState[PCState] = (HostMemoryAddress) ThreadRoot;
	machineState[StartupPCState] = (HostMemoryAddress) InterruptEnable;
	machineState[InitialPCState] = (HostMemoryAddress) func;
	machineState[InitialArgState] = (HostMemoryAddress) arg;
	machineState[WhenDonePCState] = (HostMemoryAddress) ThreadFinish;
}

//----------------------------------------------------------------------------------------
// Thread::Join
// Provoca que el thread llamante se bloquee y espere la finalizacion del thread sobre el
// cual realizo el join().
//----------------------------------------------------------------------------------------

void Thread::Join()
{
	// Comprobamos que se pueda realizar el JOIN.

	ASSERT(isJoinable);

	// Indicamos que el JOIN fue realizado.

	joinLock->Acquire();
	joinSem->V();

	// Hacemos que el thread que invoca a JOIN espere sobre la CV.

	DEBUG('j', "[THREAD-JOIN]: Waiting end of Thread %s...\n", getName());
	joinCV->Wait();
	joinLock->Release();

	// FIX: liberamos la memoria utilizada para sincronizar el JOIN. No debe
	// liberarse en el destructor del thread sobre el cual hicimos el JOIN.

	delete joinSem; joinSem = NULL; delete joinSemName;
	delete joinCV; joinCV = NULL; delete joinCVName;
	delete joinLock; joinLock = NULL; delete joinLockName;

	// Esperamos recibir el mensaje que enviara el thread sobre el cual se realizo
	// el JOIN cuando finaliza.

	//int msg;
	//joinPort->Receive(&msg);
	//const char* tname = currentThread->getName();
	//DEBUG('j', "[THREAD-JOIN]: %s received message %d.\n", tname, msg);
}

//----------------------------------------------------------------------------------------
// Thread::setPriority
// Permite setear la prioridad del thread.
//----------------------------------------------------------------------------------------

int Thread::setPriority(int p)
{
	if (p <= _MAX_PRIORITY && p >= 0)
		priority = p;
	else if (p < 0)
		priority = 0;
	else
		priority = _MAX_PRIORITY;

	return priority;
}


#ifdef USER_PROGRAM
#include "machine.h"

//----------------------------------------------------------------------------------------
// Thread::SaveUserState
// Save the CPU state of a user program on a context switch.
//
// Note that a user program thread has *two* sets of CPU registers -- one for its state
// while executing user code, one for its state while executing kernel code.
// This routine saves the former.
//----------------------------------------------------------------------------------------

void Thread::SaveUserState()
{
	for (int i = 0; i < NumTotalRegs; i++)
		userRegisters[i] = machine->ReadRegister(i);
}

//----------------------------------------------------------------------------------------
// Thread::RestoreUserState
// Restore the CPU state of a user program on a context switch.
//
// Note that a user program thread has *two* sets of CPU registers -- one for its state
// while executing user code, one for its state while executing kernel code.
// This routine restores the former.
//----------------------------------------------------------------------------------------

void Thread::RestoreUserState()
{
	for (int i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, userRegisters[i]);
}

#endif
