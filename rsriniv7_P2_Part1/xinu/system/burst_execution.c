#include <xinu.h>

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