#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

#define SYSCALL_SETWEIGHT 398
#define SYSCALL_GETWEIGHT 399

#define SCHED_WRR 7


int main(int argc, char** argv)
{
    int cpu = -1, weight = 0;
    unsigned int num = 1;
    struct sched_param param;

    cpu_set_t  mask;

    weight = atoi(argv[1]);
    cpu = atoi(argv[2]);
    
    printf("Dummy process started\n");

    param.sched_priority = 0;
    
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

if (sched_setscheduler(0, SCHED_WRR, &param)) {
    //if (syscall(SYSCALL_SCHED_SETSCHEDULER, 0 , SCHED_WRR, &param)) {
        fprintf(stderr, "Dummy process failed to change schedule policy to wrr\n");
        return 1;
    }
    printf("Dummy process schedule policy changed to WRR\n");

    if (weight) {
        if(syscall(SYSCALL_SETWEIGHT, 0, weight)) {
            fprintf(stderr, "Dummy process failed to change CPU mask %d\n", cpu);
        }
        printf("Dummy process WRR weight is now %d\n", weight);
    }

    if (cpu != -1) {
        if(sched_setaffinity(0, sizeof(mask), &mask)) {
            fprintf(stderr, "Dummy process failed to change CPU mask %d\n", cpu);
        }
    }	
    	printf("Dummy Process CPU : %d\n", cpu);
    while (1) ;

    return 0;
}
