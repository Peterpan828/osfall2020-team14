# Project2 : Weighted Round-Robin Scheduler
- Team 14
- Due: 2020-11-08 Sunday 17:00:00
- In this project we build a CPU scheduler in Tizen Linux Kernel and then we test it on an emulation of Raspberry Pi using QEMU.
- Symmetric Multiprocessor Weighted Round-Robin Scheduler:
    - Fully Supports Multiprocessort systems. 
    - Assignes to the CPU the process with the smallest weight every Time slice of 10ms, it goes back to the end of the queue after that
    - Load balancing is attempted every 2000ms and consists of keeping each CPU weight balanced by moving tasks form the runqueue with the highest weight to the lowest weight without causing it to reverse again and while respecting the fact the tunning tasks and tasks which have rrestrictions on which CPU they need to be run on shall not be moved.

## Build kernel
- ```sudo ./proj2.sh```
  By running this script our kernel will start building.

## Build Test
- ```cd test```
- ```sudo ./test.sh```

# Important pieces of code

## Preparation
- Adding ```SCHED_WRR``` as a new scheduler in ```include/uapi/linux/sched.h.```
- Adding ```struct wrr_rq``` as a new member of run queue in ```kernel/sched/sched.h```.
- Adding ```struct sched_wrr_entity``` as a new element of ```task_struct``` in ```include/linux/sched.h```.
- Implementing necessary System Calls in in ```kernel/sched/core.c```.
- Make our scheduler the default by modifying ```INIT_TASK``` in ```include/linux/init_task.h```.

- Adding functions of manipulation of WRR scheduler in the new file ```kernel/sched/wrr.c```.

### System Calls
In this project we implemented two systemcalls, Both implemented in ```/kernel/sched/core.c```
```sched_setweight``` & ```sched_getweight``` allow us to read or update the task weight transferring PID as a field of ```task_struct``` into the call's parameter

### Implement wrr_sched_class (/kernel/sched/wrr.c)
In ```wrr.c``` we construct the structure wrr_sched_classsuch as enqueue, dequeue etc...<br />

```
const struct sched_class wrr_sched_class = {
.next                   = &fair_sched_class,
.enqueue_task           = enqueue_task_wrr,
.dequeue_task           = dequeue_task_wrr,
.pick_next_task         = pick_next_task_wrr,
.yield_task		= yield_task_wrr,
.check_preempt_curr	= check_preempt_curr_wrr,
.set_curr_task		= set_curr_task_wrr,
.put_prev_task		= put_prev_task_wrr,
.switched_from		= switched_from_wrr,
.switched_to		= switched_to_wrr,

#ifdef CONFIG_SMP
.select_task_rq         = select_task_rq_wrr,
.rq_online		= rq_online_wrr,
.rq_offline		= rq_offline_wrr,
#endif

.task_tick              = task_tick_wrr,

.get_rr_interval        = get_rr_interval_wrr,
.set_cpus_allowed       = set_cpus_allowed_common,	
};
```
in addition to more functions for load balancing.

### Implementing sched_wrr_entity (/include/linux/sched.h)
If ```wrr_rq_list``` is not empty, the first element of the list is the currently running task. ```wrr_rq_list``` is one element of ```struct sched_wrr_entity``` implemented under ```task struct```.

```
struct sched_wrr_entity {
struct list_head run_list; // linked to other task_struct, or wrr_rq
unsigned int time_slice;
unsigned int weight;
};
```

### Implementing wrr_rq (/kernel/sched/sched.h)
Members of the run queue are of type:
```
struct wrr_rq 
unsigned int wrr_nr_running; // size of run queue
struct list_head wrr_rq_list; // list of current run queue
unsigned long wrr_weight_total; // total weight of current run queue
};
```

### Implementing Load balancing 
To enhance efficiency, we execute load balancing every 2000ms. it is implemented as ```wrr_load_balance``` in ```wrr.c```.
The scheduler moves a task from the queue with the biggest total weight to the smallest. Migrating tasks happens in ```wrr_load_balance```. 
Picking the processor that will perform load balancing happens in ```fair.c```.

## test Program (/test)
- Testing if weight is inherited for forked tasks in ```fork.c```
- ```trial.c``` a test program that calculates the prime factorization of a number using the naive Trial Division method
- Testing the System calls sched_setscheduler implemented in ```/kernel/sched/core.c```

# Investigation
Using only one CPU and 3 dummy process with default weight of 10, 1 Division Process
we present Time measurement according to Weight grapgh => we notice that time taken decreases tragically with higher weight

![Capture1](https://user-images.githubusercontent.com/71239705/98446797-8dd5a580-2163-11eb-9973-f38aac3bf211.PNG)

Now we record the time taken depending on the number of processes in thw same conditions of the last test we notice that runtime is smaller the more CPU's we provide

![Capture2](https://user-images.githubusercontent.com/71239705/98446798-8e6e3c00-2163-11eb-925c-6422de1c7f67.PNG)

On the 3rd Picture we demonstrate Load balancing:
after forking 4 times we have 5 processses, on the 2nd round P1 was running on CPU1 P3 and P4 on CPU2 and P2 and P5 on CPU3 wich makes total weight=10 for each CPU which is the goal of Load Balancing. 

![Capture3](https://user-images.githubusercontent.com/71239705/98446799-8f06d280-2163-11eb-8ca7-a1b3c35fa772.PNG)
