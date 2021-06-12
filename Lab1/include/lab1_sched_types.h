/*
*   lab1_sched_types.h :
*       - lab1 header file.
*       - must contains scueduler algorithm function's declations.
*
*/

#ifndef _LAB1_HEADER_H
#define _LAB1_HEADER_H

#define MAX_QUEUE 4
#define BOOSTING_PERIOD 100

typedef struct JOB {
    char name;			// process name
    int arrival_time;
    int service_time;
    int time_quantum;		// for preemptive scheduling
    struct JOB* next;		// point next job
    int tickets;		// for Lottery
} JOB;

typedef struct JOB_LIST {
    JOB* head;
    JOB* tail;
} JOB_LIST;

typedef struct Queue {
    int time_quantum;           
    JOB* jobs;			// jobs in queue for FIFO, SJF (used array to sort fast)
    JOB_LIST* list;		// jobs in queue for RR, MLFQ, Lottery (used linkedlist)
    struct Queue* next_queue;	// for lower priority queue(MLFQ)
} Queue;

int isEmpty(JOB_LIST* list);
void push(JOB_LIST* list, JOB* job);
JOB* pop(JOB_LIST* list);
void init(int job_cnt, int total_time);
void print_job(int** footprints, int job_cnt, int total_time);
int create_workload(JOB* jobs, int job_cnt);
void sort_by_arrival_time(JOB* jobs, int job_cnt);
void sort_by_service_time(JOB* jobs, int start_idx, int prev_job_end_time, int job_cnt);
void FIFO(int job_cnt);
void SJF(int job_cnt);
void RR(int job_cnt);
void MLFQ(int job_cnt, int tq_level);
void Lottery(int job_cnt);

#endif /* LAB1_HEADER_H*/



