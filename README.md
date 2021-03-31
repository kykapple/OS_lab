# OS_lab

## Lab1 - scheduling simulator

### How to use
---------------
  ```
  $ git clone https://github.com/kykapple/OS_lab.git
  $ cd lab1_sched
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



### Scheduling Algorithm Simulation
---------------
#### Workload
| Process | Arrival Time  | Service Time  |
|---------|---------------|---------------|
|    A    |      0        |       3       |
|    B    |      2        |       6       |
|    C    |      4        |       4       |
|    D    |      6        |       5       |
|    E    |      8        |       2       |

#### FIFO
![FIFO_1](https://user-images.githubusercontent.com/76088639/113154865-68ad1500-9273-11eb-85c9-0be42939ce04.PNG)

#### SJF
![SJF_1](https://user-images.githubusercontent.com/76088639/113154908-75316d80-9273-11eb-8daf-a8893f1b8760.PNG)

#### RR - time quantum 1
![RR_1](https://user-images.githubusercontent.com/76088639/113154928-7a8eb800-9273-11eb-8e52-a40955e9e3c8.PNG)

#### RR - time quantum 4
![RR_4](https://user-images.githubusercontent.com/76088639/113154935-7bbfe500-9273-11eb-893c-62c34c2437b1.PNG)

#### MLFQ - time quantum 1
![MLFQ_1](https://user-images.githubusercontent.com/76088639/113154943-7e223f00-9273-11eb-884b-5619b3afb997.PNG)

#### MLFQ - time quantum 2^i
![MLFQ_4](https://user-images.githubusercontent.com/76088639/113154948-7f536c00-9273-11eb-8bca-c1230fffec8c.PNG)

#### Lottery
![24p 완벽띠](https://user-images.githubusercontent.com/76088639/113155028-909c7880-9273-11eb-9029-d18e89efe1d9.PNG)

