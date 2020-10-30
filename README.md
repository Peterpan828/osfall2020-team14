# Project2 : Weighted Round-Robin Scheduler
- Team 14
- Due: 2020-11-05 Thursday 17:00:00

## Build kernel
- sudo ./proj2.sh

## Build Test
- cd test
- sudo ./test.sh

## To Do
Check /kernel/sched/rt.c , /kernel/sched/fair.c

### Implement syscalls (/kernel/sched/core.c)
- Line 6760
- sched_setweight
- sched_getweight

### Implement wrr_sched_class (/kernel/sched/wrr.c)
- Implement functions (enqueue, dequeue, ...)

### Implement sched_wrr_entity (/include/linux/sched.h)
- Field of struct task_struct
- line 474

### Implement wrr_rq (/kernel/sched/sched.h)
- Field of struct rq
- line 512
- Fixed valid_policy function for WRR

### Implement test Program (/test)
- trial.c
- System call sched_setscheduler (Should support WRR)
- sched_setscheduler (/kernel/sched/core.c)
- dummy process? (Compete with our workload)

### Implement Load balancing
...
