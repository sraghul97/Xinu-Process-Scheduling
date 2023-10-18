/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	insert(pid, readylist, prptr->prprio);
	resched();

	return OK;
}

syscall print_ready_list()
{
	intmask mask = disable();
	qid16 next = firstid(readylist);
	qid16 tail = queuetail(readylist);

	kprintf("\n\n_____________________________READYLIST__________________________\n");
	while(next != tail)
	{
		kprintf("%d\t", (uint32)next);
		next = queuetab[next].qnext;
	}
	kprintf("\n\n");
	
	restore(mask);
	return OK;
}
