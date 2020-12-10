#ifndef GPS_H
#define GPS_H

#include <linux/spinlock.h>

struct gps_location{
	int lat_integer;
	int lat_fractional;
	int lng_integer;
	int lng_fractional;
	int accuracy;
};

extern struct gps_location curr_gps_location;

extern spinlock_t gps_spinlock;

extern int gps_check(struct gps_location *gps_1, struct gps_location *gps_2);

#endif
