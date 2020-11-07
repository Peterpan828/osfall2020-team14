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

	//printk(KERN_INFO "Start Enqueue!!");
       	wrr_en = &p->wrr;
	list_add_tail(&(wrr_en->run_list), &(rq->wrr.rq_head));
	resched_curr(rq);

	(rq -> wrr.nr_queue)++;
	rq -> wrr.weight_sum += wrr_en -> weight;
	(rq -> nr_running)++;
	//printk(KERN_INFO "Finished Enqueue!!");

}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{	
	struct sched_wrr_entity *wrr_en = &p->wrr;
	//printk(KERN_INFO "Start Dequeue!!");
	list_del_init(&wrr_en -> run_list);
	resched_curr(rq);
	
	rq -> wrr.nr_queue--;
	rq -> wrr.weight_sum -= wrr_en -> weight;
	rq -> nr_running--;
	sched_update_tick_dependency(rq);

	//printk(KERN_INFO "Dequeue!!");

}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_en;
	struct task_struct *next_task;

	if (list_empty(&wrr_rq->rq_head)) return NULL;
	else
	{
	//printk(KERN_INFO "Pick Next Task!!");
       	wrr_en = list_first_entry(&wrr_rq->rq_head, struct sched_wrr_entity, run_list);
	wrr_set_time_slice(wrr_en);
	next_task = container_of(wrr_en, struct task_struct, wrr);
	//printk(KERN_INFO "Picked!!");
	next_task -> se.exec_start = rq_clock_task(rq);
	return next_task;
	}
}


#ifdef CONFIG_SMP
static int find_lowest_rq(struct task_struct *task)
{
	int cpu, target=-1;
	int min;
	struct rq *rq;
	struct wrr_rq *wrr_rq;

	for_each_online_cpu(cpu){
		if (cpu!=3)
		{
			rq = cpu_rq(cpu);
			wrr_rq = &rq->wrr;

			if(target == -1)
			{
				target = cpu;
				min = wrr_rq->weight_sum;
			}
			else if(wrr_rq->weight_sum<min && cpumask_test_cpu(cpu, &task->cpus_allowed))
			{
				target = cpu;
				min = wrr_rq -> weight_sum;
			}
		}
	}
	return target;
}

                                                                                                                                                                    
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	int target;
	int min_weight_sum;
	//printk(KERN_INFO "Select CPU!!");

	cpu = task_cpu(p);
	min_weight_sum = (cpu_rq(cpu)->wrr).weight_sum;

	if (p->nr_cpus_allowed == 1){
		//printk(KERN_INFO "Not Changed!!");
		return cpu;}

	rcu_read_lock();
	target = find_lowest_rq(p);
	cpu = target;

	rcu_read_unlock();
	//printk(KERN_INFO "Selected CPU");
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

	if (--wrr_en->time_slice){
		//printk(KERN_INFO "Run More!");
		return ;
	}

	//printk(KERN_INFO "No more time slice!!");
	if (wrr_rq->nr_queue == 1)
	{
		//printk(KERN_INFO "Only One!!");
		wrr_set_time_slice(wrr_en);
	}
	else
	{
		wrr_set_time_slice(wrr_en);
		list_rotate_left(&wrr_rq->rq_head); 
		set_tsk_need_resched(curr_task);
	}
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p)
{
	struct sched_wrr_entity *wrr_en = &p->wrr;
	//printk(KERN_INFO "GET_RR_INTERVAL");
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

static struct task_struct* find_migratable_task_wrr(int min_cpu, int max_cpu, struct rq *min_rq, struct rq *max_rq, unsigned long min_weight, unsigned long max_weight)
{
	struct sched_wrr_entity *wrr_en;
	struct task_struct *migratable_task = NULL;
	struct wrr_rq *wrr_rq = &max_rq->wrr;
	unsigned long migratable_task_weight = 0;
	struct task_struct *task;
	unsigned long weight;

	list_for_each_entry(wrr_en, &wrr_rq->rq_head, run_list){
		task = container_of(wrr_en, struct task_struct, wrr);
		weight = wrr_en->weight;
		if (task_running(max_rq, task)) continue;
		if (cpumask_test_cpu(min_cpu, &task->cpus_allowed)){
			if (max_weight - weight >= min_weight + weight) {
				if ((!migratable_task) || migratable_task_weight < weight) {
					migratable_task = task;
					migratable_task_weight = weight;
				}
			}
		}
	}
	return migratable_task;
}

static void migrate_task_wrr(struct task_struct *p, int min_cpu, int max_cpu)
{
	struct rq *max_rq = cpu_rq(max_cpu);
	struct rq *min_rq = cpu_rq(min_cpu);
	deactivate_task(max_rq,p,0);
	set_task_cpu(p,min_cpu);
	activate_task(min_rq, p, 0);
}

static void wrr_load_balance(void) {
	unsigned long min_weight = 0, max_weight = 0;
	int cpu, min_cpu, max_cpu;
	struct task_struct *task;
	unsigned long flags;
	struct rq *min_rq, *max_rq;
	struct rq *rq;
	struct wrr_rq  *wrr_rq;
	unsigned long curr_weight;
	
	rcu_read_lock();
	min_cpu = -1;
	max_cpu = -1;
	for_each_online_cpu(cpu) {
		if (cpu == 3) continue;
		else{
			rq = cpu_rq(cpu);
			wrr_rq = &rq->wrr;
			curr_weight = wrr_rq->weight_sum;
		
			if (min_cpu == -1 || min_weight > curr_weight) {
				min_cpu = cpu;
				min_weight = curr_weight;
			}
		
			if (max_cpu == -1 || max_weight < curr_weight) {
				max_cpu = cpu;
				max_weight = curr_weight;	
			}
		}
	}						    

	if (min_cpu == max_cpu) {
		rcu_read_unlock();
		return;
	}
		
	// now we found MAX/MIN weighted cpu.
	min_rq = cpu_rq(min_cpu);
	max_rq = cpu_rq(max_cpu);
	local_irq_save(flags);
	double_rq_lock(min_rq, max_rq);

	task = find_migratable_task_wrr(min_cpu, max_cpu, min_rq, max_rq, min_weight, max_weight);

	// we don't have to retry...
	if (task)
	{
		migrate_task_wrr(task, min_cpu, max_cpu);
		//printk(KERN_INFO "Balancing!!");
	}

	double_rq_unlock(min_rq, max_rq);
	local_irq_restore(flags);
	rcu_read_unlock();
}

// jiffies of NEXT balance time
unsigned long wrr_next_balance;
static DEFINE_SPINLOCK(wrr_balance_lock);

void wrr_trigger_load_balance(void) {
	if (!time_after_eq(jiffies, wrr_next_balance)) return;

	// for now, we should do load balancing
	spin_lock(&wrr_balance_lock);

	if (!time_after_eq(jiffies, wrr_next_balance)) {
		// LB already performed.
		spin_unlock(&wrr_balance_lock);
		return;
	}
	
	// before LB, set timeout.
	// printk(KERN_INFO "Load Balancing!!");
	wrr_next_balance = jiffies + 20;	
	spin_unlock(&wrr_balance_lock);
	wrr_load_balance();
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
	.set_cpus_allowed = set_cpus_allowed_common,

	
};

