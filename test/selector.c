#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <uapi/asm-generic/errno-base.h>
#include <unistd.h>

#define sys_set_rotation 398
#define sys_rotlock_read 399
#define sys_rotlock_write 400
#define sys_rotunlock_read 401
#define sys_rotunlock_write 402

int main(int argc, char *argv[])
{
    assert(argc == 2);
    int num = atoi(argv[1]);
    int rc;
    FILE * fp;
    
    while(1)
    {
        rc = syscall(sys_rotlock_write, 90, 90);
        if (rc<0) return -EINVAL;
        
        fp = fopen("integer", "w+"); // Make file if empty
        if(fp == NULL) printf("Cannot Open file!!\n");
        fprintf(fp, "%d", num);
        fclose(fp);
        
        printf("selector: %d\n", num);
        rc = syscall(sys_rotunlock_write, 90, 90);
        if (rc<0) return -EINVAL;
        num++;
        //sleep(1.0); // for debug
    }
    return 0;
}