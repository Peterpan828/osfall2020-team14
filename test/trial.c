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

void trial_division(int num, pid_t pid)
{	
	int weight;
	struct timespec begin, end;
	long sec, nsec;

	if (pid == 0) syscall(SCHED_SETWEIGHT, getpid(), 20);
	else syscall(SCHED_SETWEIGHT, getpid(), 1);

	weight = syscall(SCHED_GETWEIGHT, getpid());
	printf("PID : %d  Weight : %d\n", getpid(), weight);
	
	int f = 2;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	while(num > 1)
	{
		if (num%f ==0){
			//printf("New Factor is %d\n", f);
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

	if (pid == 0) printf("Child(20) : %ld.%02lds\n", sec, nsec);
	else printf("Parent(1) : %ld.%02lds\n", sec ,nsec);
}


int main(int argc, char *argv[])
{
	assert(argc == 2);
	int procs = atoi(argv[1]);
	int target = 1833357158; // random number?
	int status;
	pid_t pid;
	
	
	struct sched_param wrr_param;
	wrr_param.sched_priority = 0;
	
	sched_setscheduler(0,7,&wrr_param);
	
	for(int i=0; i<procs; i++)
	{	
		pid = fork();
		if (pid == 0) break;
	}

	trial_division(target, pid);

	for (int i=0; i<procs; i++)
	{
		wait(&status);
	}
	return 0;
}
