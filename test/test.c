#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void main(){
	FILE *fp = fopen("/home/tmp.txt","w");
	fprintf(fp, "NOOO!\n");
	printf("done!\n");
}
