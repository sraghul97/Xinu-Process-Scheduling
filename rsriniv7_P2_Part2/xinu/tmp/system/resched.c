/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <stdlib.h>

//#define DEBUG_CTXSW

struct defer Defer;

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

pid32 lottery(void)
{
	pid32 pid;
	uint32 counter = 0;
	uint32 winner;
	uint32 total_tickets = 0;
	qid16 current = firstid(lotterylist);
	qid16 tail = queuetail(lotterylist);

	// counter: used to track if we’ve found the winner yet
	// winner: use some call to a random number generator to get a value, between 0 and the total # of tickets
	// current: use this to walk through the list of jobs

	if (nonempty(lotterylist)) 
	{
		if (queuetab[firstid(lotterylist)].qnext == queuetail(lotterylist)) // lotterylist has only one element
		{
			pid = dequeue(lotterylist);
			if (proctab[pid].tickets)
			{
				return pid;
			}
			else
			{
				insert(pid, lotterylist, proctab[pid].tickets);
				return dequeue(readylist);
			}
		}
	}
	total_tickets = 0;
	while (current != tail)
	{
		total_tickets += queuetab[current].qkey;
		current = queuetab[current].qnext;
	}

	current = firstid(lotterylist);
	winner = rand() % total_tickets;

	while (current != tail)
	{
		counter += queuetab[current].qkey;
		if (counter > winner)
			return getitem(current); // ’current’ is the winner: schedule it in resched(). current "pid" is returned
		current = queuetab[current].qnext;
	}
	return SYSERR;
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void resched(void) /* Assumes interrupts are disabled	*/
{
	struct procent *prptr_old; /* Ptr to table entry for old process	*/
	struct procent *prptr_new; /* Ptr to table entry for new process	*/
	pid32 old_pid = currpid;   // Save existing process (current, to be old)

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0)
	{
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	prptr_old = &proctab[currpid];
	if (prptr_old->prstate == PR_CURR) // Process remains eligible
	{
		if (prptr_old->user_process) // Old Process -> User Processs
		{
			insert(currpid, lotterylist, prptr_old->tickets);
			if (prptr_old->prprio > queuetab[firstid(readylist)].qkey) // New Process -> User Process
			{
				currpid = lottery(); // code should run a lottery only when strictly required
				if (currpid == old_pid)
					return;
			}
			else // New Process -> System Process
				currpid = dequeue(readylist);
			prptr_old->prstate = PR_READY;
		}
		else // Old process -> System Process
		{
			if (prptr_old->prprio > firstid(readylist)) 
			{
				if (currpid == NULLPROC)
				{
					if (nonempty(lotterylist))
					{
						prptr_old->prstate = PR_READY;	// New Process -> User Process
						insert(currpid, readylist, prptr_old->prprio);
						currpid = lottery(); // code should run a lottery only when strictly required
					}
					else
						return;		// New Process -> Same old System Process
				}
				else
					return;	// New Process -> Same old System Process
			}
			else // New Process -> System Process
			{
				prptr_old->prstate = PR_READY;
				insert(currpid, readylist, prptr_old->prprio);

				if ((firstid(readylist) == NULLPROC) && nonempty(lotterylist))	// New Process -> User Process
					currpid = lottery(); // code should run a lottery only when strictly required
				else
					currpid = dequeue(readylist);	// New Process -> System Process
			}
		}
	}
	else
	{ /* Process is no longer eligible */
		if (firstid(readylist) == NULLPROC)
		{
			if (nonempty(lotterylist))
				currpid = lottery();	//code should run a lottery only when strictly required
			else
				currpid = dequeue(readylist);
		}
		else 
			currpid = dequeue(readylist);
	}

	prptr_new = &proctab[currpid];
	prptr_new->prstate = PR_CURR;
	prptr_new->num_ctxsw++;
	preempt = QUANTUM; /* Reset time slice for process	*/
#ifdef DEBUG_CTXSW
	if (old_pid != currpid)
		kprintf("ctxsw::%d-%d\n", old_pid, currpid);
#endif
	ctxsw(&prptr_old->prstkptr, &prptr_new->prstkptr);
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status resched_cntl(			/* Assumes interrupts are disabled	*/
					int32 defer /* Either DEFER_START or DEFER_STOP	*/
)
{
	switch (defer)
	{

	case DEFER_START: /* Handle a deferral request */

		if (Defer.ndefers++ == 0)
		{
			Defer.attempt = FALSE;
		}
		return OK;

	case DEFER_STOP: /* Handle end of deferral */
		if (Defer.ndefers <= 0)
		{
			return SYSERR;
		}
		if ((--Defer.ndefers == 0) && Defer.attempt)
		{
			resched();
		}
		return OK;

	default:
		return SYSERR;
	}
}
