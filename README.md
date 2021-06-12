# OS_lab

## 1. Lab1 - Scheduling Simulator
---------------
  ```
  $ git clone https://github.com/kykapple/OS_lab.git
  $ cd Lab1
  $ make
  $ ./lab1_sched
  ```
  
  
### Scheduling Algorithm Summary
---------------
#### FIFO
- Schedule a process that arrives first
- Non-preemptive

![image](https://user-images.githubusercontent.com/76088639/121769560-41da5b80-cb9f-11eb-8bec-9b1ac29bd833.png)
#### SJF
- Give a higher priority to the shortest job
- Non-preemptive

![image](https://user-images.githubusercontent.com/76088639/121769661-da70db80-cb9f-11eb-9907-147c1f54ba0f.png)
#### RR
- Runs a job for time quantum and switch to the next job
- Preemptive

![image](https://user-images.githubusercontent.com/76088639/121769571-4acb2d00-cb9f-11eb-8246-b61af8698824.png)
#### MLFQ
- Consist of multiple queues
- Each queue is assigned a different priority level
- A job that is ready to run is on a single queue

![image](https://user-images.githubusercontent.com/76088639/121769577-4e5eb400-cb9f-11eb-9d00-6dc528310350.png)
#### Lottery
- Schedule a job who wins the lottery

![image](https://user-images.githubusercontent.com/76088639/121769628-a39ac580-cb9f-11eb-918a-e618452db462.png)
## 2. Lab2 - Concurrent Data Structure
---------------
  ```
  $ git clone https://github.com/kykapple/OS_lab.git
  $ cd Lab2
  $ make
  $ ./lab2_sync -c=N -q=TQ (N = Number of vehicles, TQ = time quantum)
  ```
  
### Concurrent Issue
--------------
#### Producer/Consumer Problem
- Coarse-grained Lock vs Fine-grained Lock

![쓰레드 개수 5개, 5개일 때 coarse-fine 차이](https://user-images.githubusercontent.com/76088639/115017808-077f7580-9ef2-11eb-9831-c7f11e286de4.png)
