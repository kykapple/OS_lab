# OS_lab

## 1. Lab1 - Scheduling Simulator

### How to use
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

#### SJF
- Give a higher priority to the shortest job
- Non-preemptive

#### RR
- Runs a job for time quantum and switch to the next job
- Preemptive

#### MLFQ
- Consist of multiple queues
- Each queue is assigned a different priority level
- A job that is ready to run is on a single queue

#### Lottery
- Schedule a job who wins the lottery


## 2. Lab2 - Concurrent Data Structure

### How to use
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
