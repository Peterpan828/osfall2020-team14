#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>

#define SYSCALL_SETWEIGHT 398
#define SYSCALL_GETWEIGHT 399

#define SCHED_WRR 7


int main(int argc, char *argv[])
{
    	int cpu = -1, weight = 10;
	int procs, pid;
    	struct sched_param param;
    	cpu_set_t  mask;

	assert(argc == 4);
	cpu = atoi(argv[1]);
	weight = atoi(argv[2]);
	procs = atoi(argv[3]);
    	param.sched_priority = 0;
    
    	CPU_ZERO(&mask);
    	CPU_SET(cpu, &mask);

	sched_setscheduler(0, SCHED_WRR, &param);
        syscall(SYSCALL_SETWEIGHT, 0, weight);

        sched_setaffinity(0, sizeof(mask), &mask);
   
	for(int i=0; i<procs; i++)
	{
		pid = fork();
		if(pid==0) break;
	}

	printf("Dummy Process of CPU %d Started\n", cpu);
    	while(1);

    	return 0;
}
