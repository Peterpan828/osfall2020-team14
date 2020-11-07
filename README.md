# Project2 : Weighted Round-Robin Scheduler
- Team 14
- Due: 2020-11-08 Sunday 17:00:00
- In this project we build a CPU scheduler in Tizen Linux Kernel and then we test it on an emulation of Raspberry Pi using QEMU.
- Symmetric Multiprocessor Weighted Round-Robin Scheduler:
    - Fully Supports Multiprocessort systems. 
    - Assignes to the CPU the process with the smallest weight every Time slice of 10ms, it goes back to the end of the queue after that
    - Load balancing is attempted every 2000ms and consists of keeping each CPU weight balanced by moving tasks form the runqueue with the highest weight to the lowest weight without causing it to reverse again and while respecting the fact the tunning tasks and tasks which have rrestrictions on which CPU they need to be run on shall not be moved.

## Build kernel
- sudo ./proj2.sh
  By running this script our kernel will start building.

## Build Test
- cd test
- sudo ./test.sh

# Important pieces of code
## Preparation
- Adding SCHED_WRR as a new scheduler in include/uapi/linux/sched.h.
- Adding struct wrr_rq as a new member of run queue in kernel/sched/sched.h.
- Adding struct sched_wrr_entity as a new element of task_struct in include/linux/sched.h.
- Implementing necessary System Calls in in kernel/sched/core.c.
- Make our scheduler the default by modifying INIT_TASK in include/linux/init_task.h.

- Adding functions of manipulation of WRR scheduler in the new file 'kernel/sched/wrr.c.

### System Calls
In this project we implemented two systemcalls, Both implemented in /kernel/sched/core.c
sched_setweight & sched_getweight allow us to read or update the task weight transferring PID as a field of task_struct into the call's parameter

### Implement wrr_sched_class (/kernel/sched/wrr.c)
In wrr.c we construct the structure wrr_sched_classsuch as enqueue, dequeue etc...<br />
```
const struct sched_class wrr_sched_class = {<br />
.next       		= &fair_sched_class, <br />
.enqueue_task       = enqueue_task_wrr,<br />
.dequeue_task       = dequeue_task_wrr,<br />
.yield_task         = yield_task_wrr,<br />
.check_preempt_curr = check_preempt_curr_wrr,<br />
.pick_next_task     = pick_next_task_wrr,<br />
.put_prev_task      = put_prev_task_wrr,<br />

 #ifdef CONFIG_SMP<br />
.select_task_rq     = select_task_rq_wrr,<br />
.rq_online      = rq_online_wrr,<br />
.rq_offline      = rq_offline_wrr,<br />
 #endif

.set_curr_task      = set_curr_task_wrr,<br />
.task_tick      = task_tick_wrr,<br />
.task_fork      = task_fork_wrr,<br />
.switched_from      = switched_from_wrr,<br />
.switched_to        = switched_to_wrr,<br />
.get_rr_interval         = get_rr_interval_wrr,} in addition to more functions for load balancing.
```
/kernel/sched/rt.c 
/kernel/sched/fair.c

### Implement sched_wrr_entity (/include/linux/sched.h)
struct sched_wrr_entity {<br />
    struct list_head run_list; // linked to other task_struct, or wrr_rq<br />
    unsigned int time_slice;<br />
    unsigned int weight;<br />
};<br />
### Implement wrr_rq (/kernel/sched/sched.h)
struct wrr_rq {<br />
    unsigned int wrr_nr_running; // size of run queue<br />
    struct list_head wrr_rq_list; // list of current run queue<br />
    unsigned long wrr_weight_total; // total weight of current run queue<br />
};<br />

### Implement test Program (/test)
- trial.c
- System call sched_setscheduler (Should support WRR)
- sched_setscheduler (/kernel/sched/core.c)
- dummy process? (Compete with our workload)

### Implement Load balancing
