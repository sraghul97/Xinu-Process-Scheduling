#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime)
{
	    while(proctab[currpid].runtime<runtime);
}

int main() 
{
	uint32 IterationCount;
	pid32 prA, prB;

	kprintf("P1 Runtime,P1 Turnaroundtime,P2 Runtime,P2 Turnaroundtime\n");

	for (IterationCount=1; IterationCount <= 50; IterationCount++) {
		kprintf("======= TESTCASE %d::  2 processes - with runtime -> %d ===============\n",IterationCount, IterationCount * 100);	
	    prA = create_user_process(timed_execution, 2056, "timed_execution_1", 1, IterationCount * 100);
		prB = create_user_process(timed_execution, 2056, "timed_execution_2", 1, IterationCount * 100);
		
		set_tickets(prA, 50);
		set_tickets(prB, 50);	
		
		resume(prA);
		resume(prB);
		
		receive();	
		receive();	
		
		sleepms(50); //wait for user processes to terminate	
	    kprintf("\nprocess %d:: runtime=%d, turnaround time=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime);
        kprintf("process %d:: runtime=%d, turnaround time=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime);	
		}

	return OK;
}
