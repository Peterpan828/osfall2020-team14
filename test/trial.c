#define _GNU_SOURCE

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

#define SCHED_SETWEIGHT 398
#define SCHED_GETWEIGHT 399

void trial_division(int num)
{	
	int weight;
	struct timespec begin, end;
	long sec, nsec;

	weight = syscall(SCHED_GETWEIGHT, getpid());
	
	int f = 2;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	while(num > 1)
	{
		if (num%f ==0){
			printf("New Factor is %d\n", f);
			num /= f;
		}
		else f += 1;

	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	sec = end.tv_sec - begin.tv_sec;
	nsec = end.tv_nsec - begin.tv_nsec;
	if(nsec<0)
	{
		sec --;
		nsec += 1e9;
	}

	printf("PID : %d Weight : %d Time : %ld.%02lds\n", getpid(), weight, sec, nsec);
}


int main(int argc, char *argv[])
{
	assert(argc == 4);
	int cpu = atoi(argv[1]);
	int weight = atoi(argv[2]);
	int procs = atoi(argv[3]);
	long target = 117712234; // random number?
	int status;
	pid_t pid;

	cpu_set_t mask;
	CPU_ZERO(&mask);
	for(int i=0; i<cpu; i++){
    		CPU_SET(i, &mask);
	}	

	struct sched_param wrr_param;
        wrr_param.sched_priority = 0;

	sched_setscheduler(0,7,&wrr_param);
	syscall(SCHED_SETWEIGHT, getpid(), 10);
	sched_setaffinity(0, sizeof(mask), &mask);

	for(int i=0; i<procs; i++)
	{	
		pid = fork();
		if (pid == 0)
		{
			printf("New Dummy Process!!\n");
			while(1);
		}
			
	}

	
	syscall(SCHED_SETWEIGHT, getpid(), weight);
	trial_division(target);
	wait(&status);

	return 0;
}
