#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <asm-generic/errno-base.h>

struct gps_location{
	int lat_integer;
	int lat_fractional;
	int lng_integer;
	int lng_fractional;
	int accuracy;
};

#define GET_GPS_LOCATION 399

int main(int argc, char** argv){
	
    char *tmp;
    struct gps_location gps;

    if(argc != 2){
		printf("Need file's path\n");
		return 1;
	}
    tmp = argv[1];

    if(syscall(GET_GPS_LOCATION, tmp, &gps) != 0)
    {
        return -EFAULT;
    }

    printf("GPS : %d.%d , %d.%d Acc : %d\n", gps.lat_integer, gps.lat_fractional, gps.lng_integer, gps.lng_fractional, gps.accuracy);
    printf("https://www.google.com/maps/place/%d.%d,%d.%d\n", gps.lat_integer,gps.lat_fractional,gps.lng_integer,gps.lng_fractional);

	return 0;
}
