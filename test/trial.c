#include <stdio.h>
#include <sched.h>

#define sched_setscheduler 156


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
	int procs = argv[1];
	int weight = argv[2];

	int target = 1837158; // random number?
	
	trial_division(target);
}
