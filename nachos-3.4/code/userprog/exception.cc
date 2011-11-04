// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void myFork(int);
void myYield();
int myExec(char *);

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	char fileName[128];
	int pid;
	
    if(which == SyscallException)
    {
        switch(type)
        {
	    	case SC_Halt:
           	{
	       		DEBUG('a', "Shutdown, initiated by user program.\n");
	       		interrupt->Halt();
               	break;
            }
            case SC_Fork:
            {
               myFork(machine->ReadRegister(4));
               //machine->WriteRegister(2, result);
               break;
            }
			case SC_Yield:
			{
				myYield();
				break;
			}
			case SC_Exec:
			{
				int position = 0;
			    int arg = machine->ReadRegister(4);
			    int value;
			    while (value != NULL) {
			    	machine->ReadMem(arg, 1, &value);
			        fileName[position] = (char) value;
			        position++;
			        arg++;
			    }
				pid = myExec(fileName);
				machine->WriteRegister(2, pid);
				break;
			}
		}	
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}

// Dummy function used by myFork
void myForkHelper(int funcAddr) {
	int* stateData = (int*)funcAddr;
	for(int i=0; i<NumTotalRegs; i++)
		machine->WriteRegister(i, stateData[i]);
	delete[] stateData;
	
	currentThread->space->RestoreState();
	machine->Run();
	ASSERT(FALSE);
}

// Fork system call
void myFork(int funcAddr){
	// Check funcAddr
	if(funcAddr < 0) {
		printf("The address of this function is invalid.");
		return;
	}
	AddrSpace *addrSpace = currentThread->space;
	Thread *thread = new Thread("forked thread");	
	thread->space = addrSpace;
	
	// save PC, return, and other machine registers before fork and yield
	addrSpace->SaveState();
	int* currStateData = new int[NumTotalRegs];
	for(int i=0; i<NumTotalRegs; i++)
		currStateData[i] = machine->ReadRegister(i);
		
	currStateData[PCReg] = funcAddr;
	currStateData[NextPCReg] = funcAddr+4;		
		
	thread->Fork(myForkHelper, (int)currStateData);		
	currentThread->Yield();	
}

// Yield system call
void myYield(){
	currentThread->Yield();
}

// Exec system call
int myExec(char *file){
	int spaceID;
	OpenFile *executable = fileSystem->Open(filename);
	if(executable == NULL){
		printf("Unable to open file %s\n", filename);
		return -1;
	}
	
	AddrSpace *space;
	PCB* pcb = new PCB();
	Thread *t = new Thread("Forked process");
	space = new AddrSpace(executable);
	//WORK IN PROGRESS
	
	
	
}


