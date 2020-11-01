#include "sched.h"
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/slab.h>


// To do:
// ------------------------------------------------- 
// Implement necessary functions
// -------------------------------------------------
// hint: kernel/sched/rt.c | kernel/sched/fair.c
// -------------------------------------------------
// other interesting : 
// kernel/sched/core.c | kernel/sched/sched.h | include/uapi/linux/sched.h


// time slice function
void wrr_set_time_slice(struct sched_wrr_entity *wrr_en)
{	
	// 10ms * weight 	
	wrr_en -> time_slice = msecs_to_jiffies(wrr_en -> weight * 10);
}


static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_en;
       	wrr_en = &p->wrr;
	list_add_tail(&(wrr_en->run_list), &(rq->wrr.rq_head));

	(rq -> wrr.nr_queue)++;
	rq -> wrr.weight_sum += wrr_en -> weight;
	rq -> nr_running++;

}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_en;
	struct task_struct *next_task;
       	wrr_en = list_first_entry(&wrr_rq->rq_head, struct sched_wrr_entity, run_list);
	wrr_set_time_slice(wrr_en);
	next_task = container_of(wrr_en, struct task_struct, wrr);
	return next_task;
}

static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
}


static void task_tick_wrr(struct rq *rq, struct task_struct *task, int queued)
{
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
}



// Declared in kernel/sched/sched.h
const struct sched_class wrr_sched_class = {
        .next                   = &fair_sched_class,
        .enqueue_task           = enqueue_task_wrr,
        .dequeue_task           = dequeue_task_wrr,
        .pick_next_task         = pick_next_task_wrr,

#ifdef CONFIG_SMP
        .select_task_rq         = select_task_rq_wrr,
#endif

        .task_tick              = task_tick_wrr,

        .get_rr_interval        = get_rr_interval_wrr,

	
};

