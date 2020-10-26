# Project2 : Weighted Round-Robin Scheduler
- Team 14
- Due: 2020-11-05 Thursday 17:00:00

## Build kernel
- sudo ./proj2.sh

## Build Test
- cd test
- make all
- mv executable file to mount/root/test

## To Do

### Implement syscalls (/kernel/sched/core.c)
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

### Implement test Program (/test)
- trial.c
- System call sched_setscheduler
- dummy process? (Compete with workload)

### Implement Load balancing
...
