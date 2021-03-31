# OS_lab

## Lab1 - scheduling simulator

### How to use

  ```
  $ git clone https://github.com/kykapple/OS_lab.git
  $ cd lab1_sched
  $ make
  $ ./lab1_sched
  ```
  
### Scheduling Algorithm Summary

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

### Scheduling Algorithm Simulation

#### Workload
| Process | Arrival Time  | Service Time  |
|---------|---------------|---------------|
|    A    |      0        |       3       |
|    B    |      2        |       6       |
|    C    |      4        |       4       |
|    D    |      6        |       5       |
|    E    |      8        |       2       |

#### FIFO
<img src=">
