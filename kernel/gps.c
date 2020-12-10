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
#include<linux/limits.h>
#include<linux/path.h>
#include<linux/namei.h>
#include<linux/fs.h>

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
	printk("sys_set_gps_location\n");
	
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
	
	char *pathname_kern;
	struct path path_to_find;
	struct inode *file_inode;
	struct gps_location gps;

	printk("sys_get_gps_location\n");

	pathname_kern = kmalloc(PATH_MAX, GFP_KERNEL);

	
	// copy pathname
	if(strncpy_from_user(pathname_kern, pathname, PATH_MAX) <= 0)
	{
		kfree(pathname_kern);
		return -EFAULT;
	}
	
	// find path
	if(kern_path(pathname_kern, LOOKUP_FOLLOW, &path_to_find) < 0)
	{
		kfree(pathname_kern);
		return -EFAULT;
	}

	kfree(pathname_kern);

	// find inode
	file_inode = path_to_find.dentry -> d_inode;
	if(file_inode == NULL || file_inode -> i_op == NULL)
	{
		return -EFAULT;
	}

	// check permission
	if(generic_permission(file_inode, MAY_READ) < 0)
	{
		return -EACCES;
	}
	
	// get gps_location

	if(file_inode->i_op->get_gps_location(file_inode, &gps) < 0)
	{
		return -EFAULT;
	}

	// copy gps to loc
	if(copy_to_user(loc, &gps, sizeof(struct gps_location)))
	{
		return -EFAULT;
	}
	return 0;
}

int gps_check(struct gps_location *gps_1, struct gps_location *gps_2)
{
	// check gps_1 nears gps_2 (Inserted in fs/namei.c -> generic_permission)
	return 1;
}
