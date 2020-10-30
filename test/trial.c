#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define SCHED_SETWEIGHT 398
#define SCHED_GETWEIGHT 399

void trial_division(int num)
{	
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
	//assert(argc == 3);
	//int procs = argv[1];
	//int weight = argv[2];

	int target = 1837158; // random number?
	
	printf("Current Process's Scheduler is %d\n", sched_getscheduler(0));
	
	struct sched_param wrr_param;
	wrr_param.sched_priority = 0;
	
	printf("Return Value of syscall is %d\n", sched_setscheduler(0,7,&wrr_param));
	//perror("");
	
	printf("Current Process's Scheduler is %d\n", sched_getscheduler(0));
	trial_division(target);
}
