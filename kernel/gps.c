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
#include <linux/math64.h>
#include<linux/fs.h>

#define PRECISION 1000000000

#define APPROX_DEGREE 20

DEFINE_SPINLOCK(gps_spinlock);
struct gps_location curr_gps_location;

struct myfloat{
	long long integer;
      	long long fractional;
};

static const struct myfloat DEG_TO_RAD = {0, 17453293};
static const struct myfloat ACCURACY_TO_DEG = {0, 9014};
static const struct myfloat INVERSE_N[APPROX_DEGREE] = {
    {1, 0},
    {1, 0},
    {0, 500000000},
    {0, 333333333},
    {0, 250000000},
    {0, 166666667},
    {0, 142857143},
    {0, 125000000},
    {0, 111111111},
    {0, 100000000},
    {0, 90909091},
    {0, 83333333},
    {0, 76923077},
    {0, 71428571},
    {0, 66666667},
    {0, 62500000},
    {0, 58823529},
    {0, 55555556},
    {0, 52631579}
};

#define ONE {1, 0}
#define ZERO {0, 0}
#define MONE {-1, 0}
static const struct myfloat TAYLOR_SIN[APPROX_DEGREE] = {
    ZERO, ONE, ZERO, MONE,
    ZERO, ONE, ZERO, MONE,
    ZERO, ONE, ZERO, MONE,
    ZERO, ONE, ZERO, MONE,
    ZERO, ONE, ZERO, MONE
};
static const struct myfloat TAYLOR_COS[APPROX_DEGREE] = {
    ONE, ZERO, MONE, ZERO,
    ONE, ZERO, MONE, ZERO,
    ONE, ZERO, MONE, ZERO,
    ONE, ZERO, MONE, ZERO,
    ONE, ZERO, MONE, ZERO
};


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
	
	char *pathname_kern;
	struct path path_to_find;
	struct inode *file_inode;
	struct gps_location gps;

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

// add @rhs into @m
static void add(struct myfloat *m, const struct myfloat *rhs) {
    	m->integer += rhs->integer;
   	m->fractional += rhs->fractional;
    	if(m->fractional >= PRECISION) {
        	m->fractional -= PRECISION;
        	m->integer++;
    	}
}

// negate @m
static void neg(struct myfloat *m) {
    	m->fractional = -m->fractional;
    	m->integer = -m->integer;
    	if (m->fractional < 0) {
        	m->fractional += PRECISION;
        	m->integer--;
    	}
}

// mutiply @rhs into @m
static void multiply(struct myfloat *m, const struct myfloat *rhs) {
	long long last = m->fractional * rhs->fractional;
	long long carry;

    	m->fractional = (m->integer)*(rhs->fractional) + (m->fractional)*(rhs->integer);
    	m->integer *= rhs->integer;

    	carry = (m->fractional) < 0 ? -div64_long(-m->fractional-1, PRECISION)-1 : div64_long(m->fractional, PRECISION);
    	m->integer += carry;
    	m->fractional -= carry * PRECISION;

    	m->fractional += div64_long(last, PRECISION);
    	carry = (m->fractional) < 0 ? -div64_long(-m->fractional-1, PRECISION)-1 : div64_long(m->fractional, PRECISION);
    	m->integer += carry;
    	m->fractional -= carry * PRECISION;

    	if (last - div64_long(last, PRECISION)*PRECISION >= PRECISION/2) {
        	m->fractional++;
        	if (m->fractional >= PRECISION) {
        	    m->fractional -= PRECISION;
        	    m->integer++;
        	}
    	}
}

static void do_taylor(const struct myfloat *m, const struct myfloat *taylor, struct myfloat *out) {
    	struct myfloat acc = {1, 0};
    	int i;
    	out->integer = out->fractional = 0;
    	for(i=0;i<APPROX_DEGREE;i++){
        	struct myfloat cur = taylor[i];
        	if (cur.integer || cur.fractional) {
            		multiply(&cur, &acc);
            		add(out, &cur);
        	}

       		if(i+1<APPROX_DEGREE) {
            		multiply(&acc, m);
            		multiply(&acc, &INVERSE_N[i+1]);
        	}
    	}
}

static void mycos(const struct myfloat *m, struct myfloat *out);

static void mysin(const struct myfloat *m, struct myfloat *out) {
    	struct myfloat x = *m;
    	
	if(x.integer >= 90) {
        	x.integer -= 90;
        	mycos(&x, out);
        	return;
    	}

    	if(x.integer > 45 || (x.integer == 45 && x.fractional)) {
        	struct myfloat ninety = {90, 0};
        	neg(&x);
        	add(&x, &ninety);
        	mycos(&x, out);
        	return;
    	}

    	multiply(&x, &DEG_TO_RAD);
    	do_taylor(&x, TAYLOR_SIN, out);
}

static void mycos(const struct myfloat *m, struct myfloat *out) {
    	struct myfloat x = *m;
    	
    	if(x.integer >= 90) {
        	x.integer -= 90;
        	mysin(&x, out);
        	neg(out);
        	return;
    	}
	
    	if(x.integer > 45 || (x.integer == 45 && x.fractional)) {
        	struct myfloat ninety = {90, 0};
        	neg(&x);
        	add(&x, &ninety);
        	mysin(&x, out);
        	return;
    	}

    	multiply(&x, &DEG_TO_RAD);
    	do_taylor(&x, TAYLOR_COS, out);
}



static void spherical_cos(const struct myfloat *lat1, const struct myfloat *lng1, const struct myfloat *lat2, const struct myfloat *lng2, struct myfloat *out) {
    	struct myfloat beta = {90, 0};
    	struct myfloat gamma = {90, 0};
    	struct myfloat nlat1 = *lat1, nlat2 = *lat2;
    	struct myfloat A = *lng1;
    	struct myfloat B = *lng2;
    	struct myfloat cb, cg, sb, sg, cA;
    	struct myfloat f1, f2;

    	neg(&nlat1), neg(&nlat2);
    	add(&beta, &nlat1), add(&gamma, &nlat2);

    	neg(&B);
    	add(&A, &B);

    	if (A.integer < 0) A.integer += 360;
    	if (A.integer >= 180) {
        	struct myfloat tmp = {360, 0};
        	neg(&A);
        	add(&A, &tmp);
    	}

    	mycos(&beta, &cb); mycos(&gamma, &cg);
    	mysin(&beta, &sb); mysin(&gamma, &sg);
    	mycos(&A, &cA);

    	f1 = cb, f2 = sb;
    	multiply(&f1, &cg);
    	multiply(&f2, &sg);
    	multiply(&f2, &cA);
    	add(&f1, &f2);

    	*out = f1;
}

static int compare_myfloat(const struct myfloat *a, const struct myfloat *b) {
    	if (a->integer > b->integer) return 1;
    	if (a->integer < b->integer) return -1;
    	if (a->fractional > b->fractional) return 1;
    	if (a->fractional < b->fractional) return -1;
    	return 0;
}

int gps_check(struct gps_location *gps_1, struct gps_location *gps_2)
{
	// check gps_1 nears gps_2 (Inserted in fs/namei.c -> generic_permission)
	struct myfloat lat1, lng1, lat2, lng2;
	struct myfloat accuracy;
	struct myfloat cos_len, cos_acc;
	struct myfloat eps = {0, PRECISION/2000};
		
	lat1.integer = gps_1->lat_integer;
	lat1.fractional = (long long)gps_1->lat_fractional * (PRECISION / 1000000);
	lng1.integer = gps_1->lng_integer;
	lng1.fractional = (long long)gps_1->lng_fractional * (PRECISION / 1000000);

	lat2.integer = gps_2->lat_integer;
	lat2.fractional = (long long)gps_2->lat_fractional * (PRECISION / 1000000);
	lng2.integer = gps_2->lng_integer;
	lng2.fractional = (long long)gps_2->lng_fractional * (PRECISION / 1000000);

	accuracy.integer = gps_1->accuracy + gps_2->accuracy;
	accuracy.fractional = 0;

	multiply(&accuracy, &ACCURACY_TO_DEG);
	if(accuracy.integer >= 180) return 1;
	
	spherical_cos(&lat1, &lng1, &lat2, &lng2, &cos_len);
	mycos(&accuracy, &cos_acc);

	add(&cos_len, &eps);
	if(compare_myfloat(&cos_len, &cos_acc) != -1) return 1;		

	return 0;
}
