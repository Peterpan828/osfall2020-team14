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

#define SET_GPS_LOCATION 398

int main(int argc, char** argv){
	struct gps_location loc;
	int ret = 0;
	int lat_integer = 0;
	int lat_fractional = 0;
	int lng_integer = 0;
	int lng_fractional = 0;
	int accuracy = 0;
	char* tmp;
	int lat_frac_len = 0;
	int lng_frac_len = 0;


	if(argc != 4){
		printf("Please input\n");
		return 1;
	}
	
	tmp = strtok(argv[1], ".");
	lat_integer  = atoi(tmp);
	tmp = strtok(NULL, ".");
	lat_frac_len = strlen(tmp);
	lat_fractional = atoi(tmp);
	

	tmp = strtok(argv[2], ".");
	lng_integer = atoi(tmp);
	tmp = strtok(NULL, ".");
	lng_frac_len = strlen(tmp);
	lng_fractional = atoi(tmp);

	accuracy = atoi(argv[3]);
	if(accuracy < 0){
		printf("Invalid accuracy!\n");
		return 1;
	}

	loc.lat_integer = lat_integer;
	loc.lat_fractional = lat_fractional;
	loc.lng_integer = lng_integer;
	loc.lng_fractional = lng_fractional;
	loc.accuracy = accuracy;
	
	double temp = lat_fractional;
	for(int i=0;i<lat_frac_len;i++){
		temp *= 0.1;
	}
	double latitude;
	if(lat_integer<0)
		latitude = lat_integer - temp;
	else
		latitude = lat_integer + temp;

	
	temp = lng_fractional;
	for(int i=0;i<lng_frac_len;i++){
		temp *= 0.1;
	}

	double longitude;
	if(lng_integer<0)
		longitude = lng_integer - temp;
	else
		longitude = lng_integer + temp;


	printf("Lat: %lf\nLng: %lf\nAcc: %d\n", latitude, longitude, loc.accuracy);

	ret = syscall(SET_GPS_LOCATION, &loc);
	if(ret < 0){
		printf("error occured!\n");
		return -1;
	}

	return 0;
}
