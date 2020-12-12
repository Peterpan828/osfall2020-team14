#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv){
	if(argc != 2){
		printf("Please input file name\n");
		return 0;
	}
	char* tmp = malloc(sizeof(char)*30);
		
	strcpy(tmp, "/root/proj4/");
	strcat(tmp,argv[1]);

	FILE *fp = fopen(tmp,"w");
	fprintf(fp, "Thank you!\n");
	free(tmp);	
	return 0;
}
