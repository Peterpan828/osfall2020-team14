#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
	FILE *fp = fopen("/root/proj4/testfile","w");
	fprintf(fp, "Thank you!\n");
	
	return 0;
}
