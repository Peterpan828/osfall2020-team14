# Project3 : Rotation lock
- Team 14
- Due: 2019-12-01 Tuesday 17:00:00 KST
- In this project we implement a new Reader-writer lock based on device rotation.
 
## Lock Policy
- several readers can grab the lock at the same time (Read locks can be overlapped)
- a single writer can grab the lock at any time 

-all this while making sure to eliminate writer starvation
to do that:
- if write lock for current degree is in thw aiting list and read lock acquired current degree, no more read lock can be acquired.
- if a reader and a writer can grab the lock, thw writer will take the lock regardless of the order

## Build kernel
- ```sudo ./proj3.sh ```
By running this script our kernel will start building.

## Build Test
- ```cd test```
- ```sudo ./test.sh```

##  System calls
| Number  | System call | Role |
| ------------- | ------------- | --------------------------------------------------------- |
| 398  | set_rotation  | updates the rotation information in the kernel |
| 399  | rotlock_read  | Take a read lock using the given rotation range  |
| 400  | rotlock_write  | Take a write lock using the given rotation range  |
| 401  | rotunlock_read  | Release a read lock using the given rotation range  |
| 402  | rotunlock_write  | Release a write lock using the given rotation range  |


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
