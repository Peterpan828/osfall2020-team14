#include<linux/types.h>
#include<linux/uaccess.h>
#include<linux/spinlock_types.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<uapi/asm-generic/errno-base.h>
#include<linux/list.h>
#include<linux/signal.h>
#include<linux/slab.h>
#include<linux/gps.h>

DEFINE_SPINLOCK(gps_spinlock);
struct gps_location curr_gps_location;

int valid_location(struct gps_location loc){
	if(loc.lat_integer < -90 || loc.lat_integer > 90) return -1;
	if(loc.lat_fractional < 0 || loc.lat_fractional >= 1000000) return -1;
	if(loc.lng_integer < -180 || loc.lng_integer > 180) return -1;
	if(loc.lng_fractional < 0 || loc.lng_fractional >= 1000000) return -1;
	if(loc.lat_integer == 90 && loc.lat_fractional > 0) return -1;
	if(loc.lng_integer == 180 && loc.lng_fractional > 0) return -1;
	if(loc.accuracy < 0)return -1;
	return 1;
}

long sys_set_gps_location(struct gps_location __user *loc){
	struct gps_location temp;
	if(loc == NULL) 
		return -EINVAL;
	if(copy_from_user(&temp, loc, sizeof(struct gps_location)))
		return -EFAULT;	
	if(valid_location(temp) == -1)
	       	return -EINVAL;

	spin_lock(&gps_spinlock);
	curr_gps_location = temp;
	spin_unlock(&gps_spinlock);

	return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc){
	printk("here is get location\n");
	return 0;
}
