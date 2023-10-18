/* resched.c - resched, resched_cntl */

#include <xinu.h>
//#define DEBUG_CTXSW

struct	defer	Defer;
uint32 priority_boost_counter;

void reset_timing()
{
	ctr1000 = 0;
	priority_boost_counter = 0;
}

void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration)
{
    struct procent *process_ptr = &proctab[currpid];
    uint32 Iteration = 0;

    for(Iteration = 0; Iteration < (number_bursts+1); Iteration++)
    {
        while(process_ptr->runtime < (Iteration * burst_duration));
        sleepms(sleep_duration);
    }
}

void print_MLFQ()
{
	qid16	next, tail;

	uint32 LoopIteration = 0;
	kprintf("\n**************************************%d  %d  %d %d", UPRIORITY_QUEUES, TIME_ALLOTMENT, PRIORITY_BOOST_PERIOD, priority_boost_counter);
	for (LoopIteration = 0; LoopIteration < UPRIORITY_QUEUES; LoopIteration++)
	{
		next = firstid(mlfqlist[LoopIteration]);
		tail = queuetail(mlfqlist[LoopIteration]);
		kprintf("\nMlfq Priority Queue %d -> ", (UPRIORITY_QUEUES - LoopIteration));
		while(next != tail) 
		{
			kprintf("%d, ", (uint32)next);
			next = queuetab[next].qnext;
		}
	}
	kprintf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
}

pid32 mlfq(void) 
{
	pid32 pid, tail;
	uint32 LoopIteration = 0;
	struct procent *ptr, *oldptr;
	oldptr = &proctab[currpid];

	if (priority_boost_counter >= PRIORITY_BOOST_PERIOD) 
	{
		//print_MLFQ();
		priority_boost_counter = 0;
		
		for (LoopIteration = 0; LoopIteration < UPRIORITY_QUEUES; LoopIteration++)
		{
			if (LoopIteration == 0)
			{
				pid = firstid(mlfqlist[LoopIteration]);
				tail = queuetail(mlfqlist[LoopIteration]);		
				while (pid != tail) 
				{
					ptr = &proctab[pid];
					ptr->time_allotment = 0;
					//if(!((ptr->prstate == PR_CURR)||(ptr->prstate == PR_READY)))
						//kprintf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Not Curr/ready but in mlfqlist[%d]@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%d , %d, %d", LoopIteration, pid, ptr->prstate, ptr->prprio);
					pid = queuetab[pid].qnext;
				}
			}
			else
			{
				while (nonempty(mlfqlist[LoopIteration])) 
				{
					pid = dequeue(mlfqlist[LoopIteration]);
					ptr = &proctab[pid];
					ptr->time_allotment = 0;
					//if(!((ptr->prstate == PR_CURR)||(ptr->prstate == PR_READY)))
						//kprintf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Not Curr/ready butttt in mlfqlist[%d]@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%d , %d, %d", LoopIteration, pid, ptr->prstate, ptr->prprio);
					enqueue(pid, mlfqlist[0]);
				}
			}
		}
		pid = firstid(sleepq);
		tail = queuetail(sleepq);
		for (LoopIteration = 0; LoopIteration < NPROC; LoopIteration++)
		{
			ptr = &proctab[LoopIteration];
			if((!(ptr->prstate == PR_FREE)) && ptr->user_process)
			{
				if (UPRIORITY_QUEUES > ptr->prprio)
					ptr->upgrades += 1;
				ptr->prprio = UPRIORITY_QUEUES;
				ptr->time_allotment = 0;
			}
		}
		//kprintf("-------------------------PRIORITY BOOST DONE----------------------------%d , %d", currpid, oldptr->prstate);
		//print_MLFQ();
	}

	for (LoopIteration = 0; LoopIteration < UPRIORITY_QUEUES; LoopIteration++)
	{
		while(nonempty(mlfqlist[LoopIteration])) 
		{
			pid = dequeue(mlfqlist[LoopIteration]);
			if (LoopIteration == (UPRIORITY_QUEUES - 1))
				return pid;
			else
			{
				uint32 TimeAllotmentFactor = 1;
				uint32 LoopIteration1 = 0;
				for (LoopIteration1 = 0; LoopIteration1 < LoopIteration; LoopIteration1++)
					TimeAllotmentFactor = TimeAllotmentFactor * 2;

				ptr = &proctab[pid];
				if (ptr->time_allotment < (TIME_ALLOTMENT * TimeAllotmentFactor)) 
					return pid;
				else 
				{
					ptr->time_allotment = 0;
					ptr->downgrades += 1;
					ptr->prprio = UPRIORITY_QUEUES - (LoopIteration + 1);
					enqueue(pid, mlfqlist[LoopIteration + 1]);
				}
			}
		}
	}
	return SYSERR;
}

bool8 nonemplty_MLFQ()
{
	uint32 LoopIteration = 0;
	for (LoopIteration = 0; LoopIteration < UPRIORITY_QUEUES; LoopIteration++)
	{
		 if (nonempty(mlfqlist[LoopIteration]))
		 	return TRUE;
	}
	return FALSE;
}
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *prptr_old;	/* Ptr to table entry for old process	*/
	struct procent *prptr_new;	/* Ptr to table entry for new process	*/
	pid32 old_pid = currpid;	//Save existing process (current, to be old)

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	prptr_old = &proctab[currpid];
	if (prptr_old->prstate == PR_CURR) 	 // Process remains eligible
	{ 
		if (prptr_old->user_process) 	//Old Process -> User Processs	
		{	
			insert_MLFQ(currpid);
			if (prptr_old->prprio > queuetab[firstid(readylist)].qkey) 	//New Process -> User Process
			{
				currpid = mlfq();	//code should run a mlfq only when strictly required
				if (currpid == old_pid) 
					return;
			}
			else 	 	//New Process -> System Process
			{
				currpid = dequeue(readylist);
			}
			prptr_old->prstate = PR_READY;
		}
		else 		// Old process -> System Process
		{	
			if (prptr_old->prprio > firstid(readylist)) 	
			{	
				if (currpid == NULLPROC)
				{ 
					if (nonemplty_MLFQ())
					{
						prptr_old->prstate = PR_READY;	 //New Process -> User Process	
						insert(currpid, readylist, prptr_old->prprio);
						currpid = mlfq();	//code should run a mlfq only when strictly required
					}
					else
						return;		// New Process -> Same old System Process
				}
				else 
					return;	// New Process -> Same old System Process
			}
			else 	 	//New Process -> System Process 
			{
				prptr_old->prstate = PR_READY;
				insert(currpid, readylist, prptr_old->prprio);
				if (firstid(readylist) == NULLPROC)
				{
					if (nonemplty_MLFQ())		// New Process -> User Process
						currpid = mlfq();	//code should run a mlfq only when strictly required
					else
						currpid = dequeue(readylist);	// New Process -> System Process
				}
				else
					currpid = dequeue(readylist);	// New Process -> System Process
			}
		}
	}
	else 
	{	/* Process is no longer eligible */
		if (firstid(readylist) == NULLPROC)
		{ 
		 	if (nonemplty_MLFQ())
				currpid = mlfq();	//code should run a mlfq only when strictly required
			else
				currpid = dequeue(readylist);
		}
		else 
			currpid = dequeue(readylist);
	}


	prptr_new = &proctab[currpid];
	prptr_new->num_ctxsw += 1;
	prptr_new->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ctxsw(&prptr_old->prstkptr, &prptr_new->prstkptr);
	#ifdef DEBUG_CTXSW
		if (old_pid != currpid)
			kprintf("ctxsw::%d-%d\n", old_pid, currpid);
	#endif
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
