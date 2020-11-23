#include<linux/types.h>
#include<linux/uaccess.h>
#include<linux/spinlock.h>
#include<linux/spinlock_types.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<uapi/asm-generic/errno-base.h>
#include<linux/list.h>
#include<linux/signal.h>
#include<linux/slab.h>

#include<linux/rotation.h>

#define READ_TYPE 0
#define WRITE_TYPE 1

static LIST_HEAD(writer_waiting_list);
static LIST_HEAD(reader_waiting_list);
static LIST_HEAD(writer_active_list);
static LIST_HEAD(reader_active_list);

DEFINE_SPINLOCK(rot_spin_lock);

static int rotation = 0; // initialize rotation
static int curr_lock_state[360] = {0,};

int rot_in_bound(struct rotation_lock *param){
	int dist = rotation - param->degree;
	if(dist<0) dist = -dist;
	if(dist >= 180) dist = 360 - dist;

	return dist <= param->range;
}

int waiting_writer_check(void){
	struct rotation_lock *curr;
	int degree;
	int range;

	list_for_each_entry(curr, &writer_waiting_list, list){
		degree  = curr->degree;
		range = curr->range;
		if(rot_in_bound(curr)) return 0;
	}
	return 1;
}

static int get_lock_available(struct rotation_lock *param){
	int degree = param->degree;
	int range = param->range;
	int type = param->rw_type;

	int lower_bound = degree-range;
	if(lower_bound < 0) lower_bound += 360;
	
	int upper_bound = degree+range;
	if(upper_bound >= 360) upper_bound -= 360;
	int i;
	if(rot_in_bound(param)){
		if(type){ // writer
			for(i=lower_bound; i<lower_bound + 2*range;	i++){
				if(i >= 360) i -= 360;
				if(curr_lock_state[i] != 0) return 0;
			}
			return 1;
		}
		else{ // reader
			for(i=lower_bound;i<lower_bound+2*range;i++){
				if(i >= 360) i -= 360;
				if(curr_lock_state[i] < 0) return 0; //There is a writer that have lock
			}
			if(!waiting_writer_check()){
				return 0;	
			}
			return 1;
		}
	}
	else{
		return 0;
	}
}

int lock_active(struct rotation_lock *param){
	int degree = param->degree;
	int range = param->range;
	int i;
	if(param->rw_type == READ_TYPE){ // reader add to reader_active_list
		for(i = degree-range; i <= degree+range; i++){
			if(i>=360) i -= 360;
			if(i<0) i += 360;
		
			if(curr_lock_state[i] < 0){
				printk("lock_active failed!\n");
				return 0;
			}
			curr_lock_state[i] = 1;
		}
		list_add_tail(&param->list, &reader_active_list);
		printk("active success!!");
	}
	else{ // writer add to writer_active_list
	
	}
	return 1;
}//here is acquire lock add to list

long sys_set_rotation (int __user degree) {
	int ret = 0;

	if(degree < 0 || degree >= 360) {
		printk("Invalid degree!\n");
		return -EINVAL;
	}

	spin_lock(&rot_spin_lock);
	rotation = degree;
	//active waiting process in here!!
	ret = rotation;

	spin_unlock(&rot_spin_lock);
	return ret;
}

long sys_rotlock_read (int degree, int range) {

	printk("Here is rotlock read\n");
	struct rotation_lock *rot_lock;
	if(degree < 0 || degree >= 360){
		printk("Invalid degree!\n");
		return -EINVAL;
	}
	if(range <= 0 || range >= 180){
		printk("Invalid range!\n");
		return -EINVAL;
	}

	rot_lock = kmalloc(sizeof(struct rotation_lock), GFP_KERNEL);
	if(rot_lock == NULL){
		printk("kmalloc problem occur!\n");
		return -EFAULT;
	}
	rot_lock->degree = degree;
	rot_lock->range = range;
	rot_lock->rw_type = READ_TYPE;
	rot_lock->pid = current->pid;
	
	spin_lock(&rot_spin_lock);
	list_add_tail(&rot_lock->list, &reader_waiting_list); //reader thread add wating list
	spin_unlock(&rot_spin_lock);
	
	spin_lock(&rot_spin_lock);
	while(1){
		if(get_lock_available(rot_lock)){
			list_del(&rot_lock->list);
			lock_active(rot_lock);
			spin_unlock(&rot_spin_lock);
			return 1; // success
		}
	}
}

long sys_rotlock_write (int degree, int range){
	printk("Here is rotlock write\n");
}


struct rotation_lock *del_active_lock(int degree, int range, struct list_head *header){
	struct rotation_lock *curr;
	pid_t curr_pid = current->pid;

	list_for_each_entry(curr, &reader_active_list, list){
		if(curr->pid == curr_pid && curr->range == range && curr->degree == degree){
			list_del(&curr->list);
			return curr;
		}
	}
	return NULL;
}

int rw_lock_release(int degree, int range, int rw_type){
	int i;
	if(rw_type){ // writer

	}
	else{ // reader
		for(i=degree-range;i<=degree+range;i++){
			if(i<0) i += 360;
			if(i>=360) i -= 360;

			if(curr_lock_state[i] < 0){
				printk("Error in release lock!\n");
				return -EINVAL;
			}
			curr_lock_state[i]--;
		}
		return 1; // success!!
	}
}

long sys_rotunlock_read (int degree, int range){
	printk("here is rotunlock read");
	struct rotation_lock *rot_lock;
	int ret;
	if(degree < 0 || degree >= 360){
		printk("Invalid degree!\n");
		return -EINVAL;
	}
	if(range <= 0 || range >= 180){
		printk("Invalid range!\n");
		return -EINVAL;
	}
	spin_lock(&rot_spin_lock);
	rot_lock = del_active_lock(degree, range, &reader_active_list);

	if(rot_lock == NULL){
		spin_unlock(&rot_spin_lock);
		printk("There is no readlock\n");
		return -EINVAL;
	}

	ret = rw_lock_release(degree, range, READ_TYPE);
	if(ret != 1){
		spin_unlock(&rot_spin_lock);
		kfree(rot_lock);
		printk("Error in lock_release!\n");
		return -EFAULT;
	}
	spin_unlock(&rot_spin_lock);
	kfree(rot_lock);
	printk("success reader unlock!\n");
	return 1;
}

long sys_rotunlock_write(int degree, int range){
	printk("here is rotunlock write\n");
}
