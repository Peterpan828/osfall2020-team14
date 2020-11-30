# Project3 : Rotation lock

- Team 14
- Due: 2019-12-01 Tuesday 17:00:00 KST
- In this project we implement a new Reader-writer lock based on device rotation.
 
## Lock Policy

- several readers can grab the lock at the same time (Read locks can be overlapped)
- a single writer can grab the lock at any time 

- all this while making sure to eliminate writer starvation
to do that:
- if write lock for current degree is in thw aiting list and read lock acquired current degree, no more read lock can be acquired.
- if a reader and a writer can grab the lock, thw writer will take the lock regardless of the order

## Build kernel

- ```sudo ./proj3.sh ```
By running this script our kernel will start building.

## Build Test

- ```cd test```
- ```sudo ./test.sh```

## Preparation

- Creating 2 list_head one for the aquired list and the other is a waiting list.
- Implementing the structure rotation_lock to store info about the lock in ```/include/linux/rotation.h``` .
- Implementing 5 system calls  set_rotation, rotlock_read, rotlock_write, rotunlock_read an rotunlock_write.
- Implementing the funtion exit_lock to release all the locks in the end in ```/include/linux/rotation.c``` and  ```/kernel/exit.c``` 


## Implementation

###  System calls

all rotation related functions are implemented in 
```kernel/rotation.c```  
```include/linux/rotation.h```

| Number  | System call | Role |
| ------------- | ------------- | --------------------------------------------------------- |
| 398  | set_rotation  | updates the rotation information in the kernel |
| 399  | rotlock_read  | Take a read lock using the given rotation range  |
| 400  | rotlock_write  | Take a write lock using the given rotation range  |
| 401  | rotunlock_read  | Release a read lock using the given rotation range  |
| 402  | rotunlock_write  | Release a write lock using the given rotation range  |


###  Implement struct rotation_lock in  // include/linux/rotation.h

```
struct rotation_lock{
        int degree;
        int range;
        int rw_type; // reader or writer
        pid_t pid;
        struct list_head list;
    };
```
implemented in ```/include/linux/rotation.h```to store informations about rotational lock

###  Implement rotation related functions in  // kernel/rotation.c

Containing the impleentation of all functions related to our lock such as
- initializing the rotation, checking the crurrent lock state, checking if rotation is in bound, 
- ```waiting_writer_check()``` to periodically ckeck if there is any wrters waiting
- ```get_lock_available() ``` to check if the lock is available
- ```lock_active() ``` to acquire the lock and add the process to the list
- ```wake_next() ``` to wake the next process in line, if there is a waiting writer it has to be woken up first
- ```exit_rotlock() ``` to release all the acquired locks in the end

## Test :

- ```selector.c``` takes the write lock, writes the integer from argument to a file called integer in the current directory and then releases the lock
after that t could racquire the lock to write the same integer +1 to the same file, program will run until terminated by the user. program will print a standard output before being terminated.

- ```trial.c``` acquire the read lock, open the file called integer and calculate the prime number factorization of the integer, and then write in standard output. after that it will close the file and release the lock. this program will run until terminated by user

``` 
    ./rotd                 //deamon acts like a clock and periodically changes rotations
    ./selector 200 & 
    ./trial 0 & 
    ./trial 1
```
for our test we will run the selector first and then the trial program twice with two different identificators in 3 different windows.
