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
	printk(KERN_INFO "Start Enqueue!!");
       	wrr_en = &p->wrr;
	list_add_tail(&(wrr_en->run_list), &(rq->wrr.rq_head));

	(rq -> wrr.nr_queue)++;
	rq -> wrr.weight_sum += wrr_en -> weight;
	(rq -> nr_running)++;
	printk(KERN_INFO "Finished Enqueue!!");

}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{	
	struct sched_wrr_entity *wrr_en = &p->wrr;
	printk(KERN_INFO "Start Dequeue!!");
	list_del(&wrr_en -> run_list);
	
	(rq -> wrr.nr_queue)--;
	rq -> wrr.weight_sum -= wrr_en -> weight;
	(rq -> nr_running)--;
	printk(KERN_INFO "Dequeue!!");

}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_en;
	struct task_struct *next_task;
	printk(KERN_INFO "Pick Next Task!!");
       	wrr_en = list_first_entry(&wrr_rq->rq_head, struct sched_wrr_entity, run_list);
	wrr_set_time_slice(wrr_en);
	next_task = container_of(wrr_en, struct task_struct, wrr);
	printk(KERN_INFO "Picked!!");
	return next_task;
}

static DEFINE_PER_CPU(cpumask_var_t, local_cpu_mask);



#ifdef CONFIG_SMP
static int find_lowest_rq(struct task_struct *task)
{
        struct sched_domain *sd;
        struct cpumask *lowest_mask = this_cpu_cpumask_var_ptr(local_cpu_mask);
        int this_cpu = smp_processor_id();
        int cpu      = task_cpu(task);

        /* Make sure the mask is initialized first */
        if (unlikely(!lowest_mask))
                return -1;

        if (task->nr_cpus_allowed == 1)
                return -1; /* No other targets possible */

        if (!cpupri_find(&task_rq(task)->rd->cpupri, task, lowest_mask))
                return -1; /* No targets found */

        /*
         * At this point we have built a mask of cpus representing the
         * lowest priority tasks in the system.  Now we want to elect
         * the best one based on our affinity and topology.
         *
         * We prioritize the last cpu that the task executed on since
         * it is most likely cache-hot in that location.
         */
        if (cpumask_test_cpu(cpu, lowest_mask))
                return cpu;

        /*
         * Otherwise, we consult the sched_domains span maps to figure
         * out which cpu is logically closest to our hot cache data.
         */
        if (!cpumask_test_cpu(this_cpu, lowest_mask))
                this_cpu = -1; /* Skip this_cpu opt if not among lowest */

        rcu_read_lock();
        for_each_domain(cpu, sd) {
                if (sd->flags & SD_WAKE_AFFINE) {
                        int best_cpu;

                        /*
                         * "this_cpu" is cheaper to preempt than a
                         * remote processor.
                         */
                        if (this_cpu != -1 &&
                            cpumask_test_cpu(this_cpu, sched_domain_span(sd))) {
                                rcu_read_unlock();
                                return this_cpu;
                        }

                        best_cpu = cpumask_first_and(lowest_mask,
                                                     sched_domain_span(sd));
                        if (best_cpu < nr_cpu_ids) {
                                rcu_read_unlock();
				return best_cpu;
                        }
                }
        }
        rcu_read_unlock();

        /*
         * And finally, if there were no matches within the domains
         * just give the caller *something* to work with from the compatible
         * locations.
         */
        if (this_cpu != -1)
                return this_cpu;

        cpu = cpumask_any(lowest_mask);
        if (cpu < nr_cpu_ids)
                return cpu;
        return -1;
}

                                                                                                                                                                    
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	int target;
	int min_weight_sum;
	printk(KERN_INFO "Select CPU!!");

	cpu = task_cpu(p);
	min_weight_sum = (cpu_rq(cpu)->wrr).weight_sum;

	if (p->nr_cpus_allowed == 1){
		printk(KERN_INFO "Not Changed!!");
		return cpu;}

	rcu_read_lock();
	target = find_lowest_rq(p);
	cpu = target;

	rcu_read_unlock();
	printk(KERN_INFO "Selected CPU");
	return cpu;
}

static void rq_online_wrr(struct rq *rq)
{

}

static void rq_offline_wrr(struct rq *rq)
{

}
#endif

static void task_tick_wrr(struct rq *rq, struct task_struct *task, int queued)
{
	struct task_struct *curr_task = rq->curr;
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_en = &curr_task->wrr;

	if (wrr_en->time_slice > 0){
		printk(KERN_INFO "Run More!");
		return ;}

	printk(KERN_INFO "No more time slice!!");
	if (wrr_rq->nr_queue == 1)
		wrr_set_time_slice(wrr_en);
	else
	{
		list_rotate_left(&wrr_rq->rq_head); 
		set_tsk_need_resched(curr_task);
	}


}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
	struct sched_wrr_entity *wrr_en = &p->wrr;
	printk(KERN_INFO "GET_RR_INTERVAL");
	return msecs_to_jiffies(wrr_en->weight *10);
}

static void yield_task_wrr(struct rq *rq)
{

}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{

}

static void set_curr_task_wrr(struct rq *rq)
{
	//printk(KERN_INFO "set_curr_task called");
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{

}

static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
	// Prevent Error
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	// Prevent Error
}

// Declared in kernel/sched/sched.h
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

	
};

