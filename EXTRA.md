* 1\. The Load Balancing Cost seems to be a little expensive, so the weight difference during Balancing must exceed a certain threshold for migration to happen

* 2\. If we move a task through Load balancing to the end of a certain queue and it causes reverse load inbalance, This task will be in constant movement.
A solution to this problem is: when we move tasks we could put them in the middle of the recieving queue or prioritize moing the non-running tasks that have the smallest weight. 

* 3\. In round robin algorithm the performance mainly depends on the size of the quantum, small quantum gives poor performance and larger one makes RR work like FIFO. thats why choosing quantum size is very critical to enhance system performance.

* 4\. Delay sensitive services (ex. Voice services..) may not work properly if the weight or Queue priority are low. To fix this issue, we can implement a systemcall that changes the order of the Queue for load where response time is important.
 
* 5\. If there is many tasks whose Time-slice is shorter than migration time, than maybe migrating these tasks could be unuseful. By having a CPU that is aware of the average task time, we can grasp the task's time slice and adjust the frequece of the load balance operation accordingly


