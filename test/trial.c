#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uapi/asm-generic/errno-base.h>

#define sys_set_rotation 398
#define sys_rotlock_read 399
#define sys_rotlock_write 400
#define sys_rotunlock_read 401
#define sys_rotunlock_write 402

void trial_division(int num)
{	
	
	int f = 2;

	while(num > 1)
	{
		if (num%f ==0){
			//printf("New Factor is %d\n", f);
			printf("% d ", f);
			num /= f;
			if (num!=1) printf("*");
		}
		else f += 1;

	}
	printf("\n");
	
}

int main(int argc, char *argv[])
{
    assert (argc == 2);
	int idx = atoi(argv[1]);
	FILE * fp;
	int num;
	int rc;

	while(1)
	{ 	
		//printf("Try lock\n");
		rc = syscall(sys_rotlock_read, 90, 90);
        if (rc<0) return -EINVAL;
		//printf("Acquired lock\n");

		fp = fopen("integer", "r");
        if(fp == NULL) printf("Cannot Open file!!\n");
        fscanf(fp, "%d", &num);
        fclose(fp);

		printf("trial-%d: %d =", idx, num);
		trial_division(num);

		//printf("Try release\n");
		rc = syscall(sys_rotunlock_read, 90, 90);
        if (rc<0) return -EINVAL;
		//printf("Released\n");
		//sleep(1.0); // for debug
	}
}