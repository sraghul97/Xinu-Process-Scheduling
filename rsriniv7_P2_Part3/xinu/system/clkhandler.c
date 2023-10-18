/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/
	struct	procent	*prptr;

	ctr1000+=1;
	priority_boost_counter +=1;
	prptr = &proctab[currpid];
	prptr->runtime++;
	if (prptr->user_process)
		prptr->time_allotment++;
	/* Decrement the ms counter, and see if a second has passed */

	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		preempt = QUANTUM;
		resched();
	}

	//if(priority_boost_counter >= PRIORITY_BOOST_PERIOD) 
	//{
	//	kprintf("\n***************priority_boost_counter >= PRIORITY_BOOST_PERIOD***********************%d  %d  %d %d", UPRIORITY_QUEUES, TIME_ALLOTMENT, PRIORITY_BOOST_PERIOD, priority_boost_counter);
	//	resched();
	//}
}
