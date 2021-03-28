/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32170260
*	    Student name : Kweon Yeong Kee
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm function'definition.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab1_sched_types.h"

int** footprints = NULL;

int isEmpty(JOB_LIST* list) {
	return list->head == NULL ? 1 : 0;  // if empty return 1
}

void push(JOB_LIST* list, JOB* job) {
	job->next = NULL;
	if (list->head == NULL) {
		list->head = job;
	}
	else {
		list->tail->next = job;
	}
	list->tail = job;
}

JOB* pop(JOB_LIST* list) {
	if (isEmpty(list)) {
		printf("Ready queue is Empty\n");
	return NULL;
	}

	JOB* job = list->head;
	list->head = list->head->next;

	return job;                     // return current job
}

void init(int job_cnt, int total_time) {
	if (footprints == NULL) {
		footprints = (int**)malloc(sizeof(int*) * job_cnt);

		for (int i = 0; i < job_cnt; i++)
			footprints[i] = (int*)malloc(sizeof(int) * total_time);
	}

	for (int i = 0; i < job_cnt; i++) {
		for (int j = 0; j < total_time; j++) {
			footprints[i][j] = 0;
		}
	}
}

void print_job(int** footprints, int job_cnt, int total_time) {
	for (int i = 0; i < job_cnt; i++) {
		printf("%c ", 'A' + i);
 		for (int j = 0; j < total_time; j++) {
			if (footprints[i][j] == 1)
				printf("■ "); 
			else
				printf("□ ");
			}
		printf("\n");
	}

	printf("\n\n");
}

int create_workload(JOB* jobs, int job_cnt) {
	int total_service_time = 0;
	
	for (int i = 0; i < job_cnt; i++) {
		jobs[i].name = 'A' + i;
		printf("Enter job %c's Arrival Time and Service TIme: ", jobs[i].name);
		scanf("%d %d", &jobs[i].arrival_time, &jobs[i].service_time);
		total_service_time += jobs[i].service_time;
		jobs[i].time_quantum = 0;			// for MLFQ scheduling
	}

	return total_service_time;
}

void sort_by_arrival_time(JOB* jobs, int job_cnt) {
	for (int i = 0; i < job_cnt - 1; i++) {
		int idx = i;

		for (int j = i + 1; j < job_cnt; j++) {
			if (jobs[idx].arrival_time > jobs[j].arrival_time)		// sort by arrival time
				idx = j;
			else if (jobs[idx].arrival_time == jobs[j].arrival_time) {	// tie-break rule
				if (jobs[idx].name > jobs[j].name)
					idx = j;
			}

			JOB temp = jobs[idx];
			jobs[idx] = jobs[i];
			jobs[i] = temp;
		}
	}
}

void sort_by_service_time(JOB* jobs, int start_idx, int prev_job_end_time, int job_cnt) {
    for (int i = start_idx; i < job_cnt; i++) {
        if (jobs[i].arrival_time > prev_job_end_time) continue;
        int idx = i;

        for (int j = i + 1; j < job_cnt; j++) {
            if (jobs[j].arrival_time > prev_job_end_time) continue;

            if (jobs[idx].service_time > jobs[j].service_time)		// sort by service time
                idx = j;
            else if (jobs[idx].service_time == jobs[j].service_time) {	// tie-break rule
                if (jobs[idx].name > jobs[j].name)
                    idx = j;
            }

	}
            
	JOB temp = jobs[idx];
	jobs[idx] = jobs[i];
	jobs[i] = temp;
    }
}

/*
FIFO scheduling - Non-preemptive, Single Queue
*/
void FIFO(int job_cnt) {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->jobs = (JOB*)malloc(sizeof(JOB) * job_cnt);		// workload
	int total_time = create_workload(q->jobs, job_cnt);		// create workload, get total service time
    
	init(job_cnt, total_time);					// initialize footprints

	sort_by_arrival_time(q->jobs, job_cnt);			// sort by arrival time or sort alphabetically if processes arrive at the same time

	int current_time = 0;
	for (int i = 0; i < job_cnt; i++) {
		JOB* cur_job = &q->jobs[i];

		while (cur_job->service_time--) {
			footprints[cur_job->name - 'A'][current_time++] = 1;	// run state
		}
	}

	printf("-------------------FIFO-------------------\n");
	print_job(footprints, job_cnt, total_time);			// simulate FIFO scheduling

	free(q->jobs);
}

/*
SJF scheduling - Non-preemptive, Single Queue, jobs that finish quickly have high priority
*/
void SJF(int job_cnt) {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->jobs = (JOB*)malloc(sizeof(JOB) * job_cnt);		// workload
	int total_time = create_workload(q->jobs, job_cnt);		// create workload, get total service time

	init(job_cnt, total_time);					// initialize footprints

	sort_by_service_time(q->jobs, 0, 0, job_cnt);			// sort by arrival time and service time to select first job

	int current_time = 0;
	for (int i = 0; i < job_cnt; i++) {
		JOB* cur_job = &q->jobs[i];

		while (cur_job->service_time--) {
			footprints[cur_job->name - 'A'][current_time++] = 1;	// run state
		}

		sort_by_service_time(q->jobs, i + 1, current_time, job_cnt);	// sort by service time to select next job
	}

	printf("-------------------SJF-------------------\n");
	print_job(footprints, job_cnt, total_time);
    
	free(q->jobs);
}

/*
RR scheduling - preemptive, job runs during the time quantum, expected RR need a linkedlist
*/
void RR(int job_cnt) {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->list = (JOB_LIST*)malloc(sizeof(JOB_LIST));	// create linkedlist
	q->list->head = NULL;
	q->list->tail = NULL;   
	
	printf("Enter time quantum: ");			// set time quantum
	scanf("%d", &q->time_quantum);

	JOB* jobs = (JOB*)malloc(sizeof(JOB) * job_cnt);	// to create workload
	int total_time = create_workload(jobs, job_cnt);	// create workload, get total service time

	init(job_cnt, total_time);				// initialize footprints
    
	sort_by_arrival_time(jobs, job_cnt);			// basically, the process that arrives first is performed first

	push(q->list, &jobs[0]);				// put first job into list

	int next = 1;
	while (jobs[next - 1].arrival_time == jobs[next].arrival_time) {	// check if there are any jobs arriving at the same time
		push(q->list, &jobs[next++]);
	}

	int current_time = 0;                   
	while (!isEmpty(q->list)) {
		JOB* cur_job = pop(q->list);			// select job
		cur_job->time_quantum = q->time_quantum;	// allocate time quantum

		while (cur_job->time_quantum--) {
			cur_job->service_time--;
			if (cur_job->service_time >= 0) {
				footprints[cur_job->name - 'A'][current_time++] = 1;	// run state
			}
			else
				break;

			if (current_time == jobs[next].arrival_time)	// push arrived job
				push(q->list, &jobs[next++]);

		}

		if (cur_job->service_time > 0)			// cases with remaining time quantum
			push(q->list, cur_job);
	}

	printf("---------------RR with tq %d---------------\n", q->time_quantum);
	print_job(footprints, job_cnt, total_time);

	free(q->list);
	free(jobs);
}

/*
MLFQ - multiple priority queues, boosting
*/
void MLFQ(int job_cnt, int tq_level) {
	int flag = 0;

	Queue** q = (Queue**)malloc(sizeof(Queue*) * MAX_QUEUE);
	for (int i = 0; i < MAX_QUEUE; i++) {
		q[i] = (Queue*)malloc(sizeof(Queue));			// create multi queue
		q[i]->list = (JOB_LIST*)malloc(sizeof(JOB_LIST));
		q[i]->list->head = NULL;
		q[i]->list->tail = NULL;
	}

	JOB* jobs = (JOB*)malloc(sizeof(JOB) * job_cnt);		// to create workload
	int total_time = create_workload(jobs, job_cnt);		// create workload, get total service time

	init(job_cnt, total_time);					// initialize footprints

	for (int i = 0; i < MAX_QUEUE; i++) {
		q[i]->time_quantum = 1;
		for (int j = 0; j < i && j < tq_level - 1; j++)	// setting time quantum
			q[i]->time_quantum *= 2;

		// link lower priority queue
		if (i < MAX_QUEUE - 1)
			q[i]->next_queue = q[i + 1];
		else
			q[i]->next_queue = NULL;
	}

	sort_by_arrival_time(jobs, job_cnt);				// sort by arrival time
	push(q[0]->list, &jobs[0]);					// put first job into the topmost queue

	int next = 1;
	while (jobs[next - 1].arrival_time == jobs[next].arrival_time) {	// check if there are any jobs arriving at the same time
		push(q[0]->list, &jobs[next++]);
	}

	int current_time = 0;
	while (current_time < total_time) {
		Queue* mq = q[0];
		while (mq && isEmpty(mq->list)) {			// select queue
			mq = mq->next_queue;
		}

		JOB* cur_job = pop(mq->list);
		if(cur_job->time_quantum <= 0)
			cur_job->time_quantum = mq->time_quantum;	// allocate time quantum
		while (cur_job->time_quantum--) {
			cur_job->service_time--;
			if (cur_job->service_time >= 0) {
				footprints[cur_job->name - 'A'][current_time++] = 1;	// run state
			}
			else {
				break;							// terminated
			}

			if (jobs[next].arrival_time == current_time) {
				push(q[0]->list, &jobs[next++]);	// newly arrived job goes to queue 0
			}
			
			flag = 0;	// for checking if all queues are empty
			for(Queue* seekq = q[0]; seekq; seekq = seekq->next_queue) {
				if(!isEmpty(seekq->list)) {
					flag = 1;
					break;
				}
			}
			
			if(!flag) {	// if all queues are empty
				cur_job->time_quantum = mq->time_quantum;	// cotinue to run same process
				continue;
			}

			// boosting
			if (current_time && current_time % BOOSTING_PERIOD == 0) {	// if it is a boosting period
				Queue* seekq = q[1];			// seek jobs starting from the queue 1
				while (seekq) {
					while (!isEmpty(seekq->list)) {
						push(q[0]->list, pop(seekq->list));	// move all the jobs to the topmost queue
					}
					seekq = seekq->next_queue;			// move to lower queue
				}
			}
        	}
        	
        	if(cur_job->service_time > 0 && cur_job->time_quantum > 0) {			// if the time quantum is not used up, keep its priority
        		push(mq->list, cur_job);
        		continue;
        	}

		if (flag) {						// means some queues have jobs
			if (cur_job->service_time > 0) {
				if (!mq->next_queue) {			// if mq is last level queue
					push(mq->list, cur_job);
				}
				else {
					push(mq->next_queue->list, cur_job);	// push job into lower queue
				}
			}
		}
	}

	if(tq_level == 1)
		printf("--------------MLFQ with tq 1--------------\n");
	else
		printf("-------------MLFQ with tq 2^i-------------\n");
	print_job(footprints, job_cnt, total_time);

	for (int i = 0; i < MAX_QUEUE; i++) {
		free(q[i]->list);
		free(q[i]);
	}
	free(q);
	free(jobs);
}

/*
Lottery scheduling - tickets, schedule a job who wins the lottery
*/
void Lottery(int job_cnt) {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->list = (JOB_LIST*)malloc(sizeof(JOB_LIST));
	q->list->head = NULL;
	q->list->tail = NULL;

	JOB* jobs = (JOB*)malloc(sizeof(JOB) * job_cnt);		// for workload
	int total_time = create_workload(jobs, job_cnt);		// create workload, get total service time

	init(job_cnt, total_time);					// initialize footprints

	for (int i = 0; i < job_cnt; i++) {                     
		push(q->list, &jobs[i]);				// push workload 
	}
    
	int total_tickets = 0;
	for (JOB* cur_job = q->list->head; cur_job; cur_job = cur_job->next) {	// set each job's ticket
		printf("Enter %c's tickets : ", cur_job->name);
		scanf("%d", &cur_job->tickets);
		total_tickets = total_tickets + cur_job->tickets;
	}

	int current_time = 0;
	srand(time(NULL));
	while (current_time < total_time) {
		int lottery = rand() % total_tickets;			// generate random value between 0 and the total # of tickets
		int sum = 0;

		JOB* cur_job = q->list->head;
		while (cur_job) {
			sum = sum + cur_job->tickets;
			if (sum > lottery)
				break;
			cur_job = cur_job->next;
		}

		if (cur_job->service_time > 0 && cur_job->arrival_time <= current_time) {
			cur_job->service_time--;
			footprints[cur_job->name - 'A'][current_time++] = 1;		// run state
		}
	}
	
	printf("-----------------Lottery-----------------\n");
	print_job(footprints, job_cnt, total_time);

	free(jobs);
	free(q->list);
	free(q);
}
