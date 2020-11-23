#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(){
	int i;
	i = syscall(398,100);
	printf("i = %d\n",i);
	printf("%d\n",syscall(399,100,50));
	printf("%d\n",syscall(400,100,100));
	printf("%d\n",syscall(401,100,50));
	printf("%d\n",syscall(402,100,100));
	return 0;
}
