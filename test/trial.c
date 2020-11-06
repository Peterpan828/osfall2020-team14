#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define SCHED_SETWEIGHT 398
#define SCHED_GETWEIGHT 399

void trial_division(int num, pid_t pid)
{	
	int weight;
	weight = syscall(SCHED_GETWEIGHT, getpid());
	printf("PID : %d  Weight : %d\n", getpid(), weight);

	int f = 2;
	while(num > 1)
	{
		if (num%f ==0){
			printf("New Factor is %d\n", f);
			num /= f;
		}
		else f += 1;

	}
}


int main(int argc, char *argv[])
{
	assert(argc == 2);
	int procs = atoi(argv[1]);
	int target = 1837158; // random number?
	pid_t pid;
	
	//printf("Current Process's Scheduler is %d\n", sched_getscheduler(0));
	
	struct sched_param wrr_param;
	wrr_param.sched_priority = 0;
	
	sched_setscheduler(0,7,&wrr_param);
	
	//printf("Current Process's Scheduler is %d\n", sched_getscheduler(0));

	for(int i=0;i<procs;i++)
	{
		pid = fork();
		if(pid < 0)	printf("Error\n");
	}

	trial_division(target, getpid());
	return 0;
}
