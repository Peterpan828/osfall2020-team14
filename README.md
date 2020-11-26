## Build :
    sudo ./proj3.sh
    cd test; sudo ./test.sh

##  System call : 
    398 399 400 401 402 (set_rotation, rotlock_read, rotlock_write, rotunlock_read, rotunlock_write)


##  Structure :
    struct rotation_lock{
        int degree;
        int range;
        int rw_type; // reader or writer
        pid_t pid;
        struct list_head list;
    }; // include/linux/rotation.h


## Test :
    ./rotd
    ./selector 200 & ./trial 0 & ./trial 1
