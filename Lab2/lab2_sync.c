/*
*   Lab2 (Vehicle production Problem)
*
*   lab2_sync.c :
*       - lab2 main file.
*       - must contains Vehicle production Problem function's declations.
*
*/

#include <aio.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab2_sync_types.h"

/*
 * you need to implement Vehicle production Problem. 
 */
 
// producers and consumers share the queue
CQ* car_queue = NULL;		// Without lock
CQ* coarse_car_queue = NULL;	// Coarse-grained Lock
CQ* fine_car_queue = NULL;	// Fine-grained Lock
int total_car;


void print(CQ* current_queue) {
	Node* cur = current_queue->front;
	while(cur) {
		printf("%d ", cur->car_num);
		cur = cur->next;
	}
	printf("\n");
}


int isEmpty(CQ* current_queue) {
	return current_queue->front == NULL ? 1 : 0;	// if queue is empty return 1
}

// Without Lock initializer
void Init() {
	car_queue = (CQ*) malloc(sizeof(CQ));	// initialize car_queue
	car_queue->balance = 0;
	car_queue->produce_number = 0;
	car_queue->front = NULL;
	car_queue->rear = NULL;
}

// Coarse-grained Lock initializer
void cg_Init() {
	coarse_car_queue = (CQ*) malloc(sizeof(CQ));	// initialize coarse_car_queue
	coarse_car_queue->balance = 0;
	coarse_car_queue->produce_number = 0;
	coarse_car_queue->front = NULL;
	coarse_car_queue->rear = NULL;
	
	pthread_cond_init(&cg_fill, NULL);
	pthread_cond_init(&cg_empty, NULL);
	
	pthread_mutex_init(&coarse_car_queue->mutex, NULL);
}

// Fine-grained Lock initializer
void fg_Init() {
	fine_car_queue = (CQ*) malloc(sizeof(CQ));	// initialize fine_car_queue
	fine_car_queue->balance = 0;
	fine_car_queue->produce_number = 0;
	fine_car_queue->front = NULL;
	fine_car_queue->rear = NULL;
	
	pthread_cond_init(&fg_fill, NULL);
	pthread_cond_init(&fg_empty, NULL);
	
	pthread_mutex_init(&fine_car_queue->mutex, NULL);
	
	pthread_mutex_init(&fine_car_queue->headLock, NULL);
	pthread_mutex_init(&fine_car_queue->tailLock, NULL);
}

// Without Lock, Coarse-grained Lock Enqueue
void Enqueue(CQ* current_queue, int car_num) {
	Node* newNode = (Node*) malloc(sizeof(Node));
	assert(newNode != NULL);
	
	newNode->car_num = car_num;
	newNode->next = NULL;
	
	if(current_queue->front == NULL)
		current_queue->front = newNode;
	else
		current_queue->rear->next = newNode;
	
	current_queue->rear = newNode;
	current_queue->balance++;
}

// Without Lock, Coarse-grained Lock Dequeue
int Dequeue(CQ* current_queue) {
	Node* delNode = current_queue->front;
	current_queue->front = current_queue->front->next;
	
	current_queue->balance--;
	int car_num = delNode->car_num;
	free(delNode);

	return car_num;
}

// Fine-grained Lock Enqueue
void fg_Enqueue(int car_num) {
	Node* newNode = (Node*) malloc(sizeof(Node));
	assert(newNode != NULL);
	
	newNode->car_num = car_num;
	newNode->next = NULL;
	
	if(fine_car_queue->front == NULL)
		fine_car_queue->front = newNode;
	else
		fine_car_queue->rear->next = newNode;
	
	fine_car_queue->rear = newNode;
	fine_car_queue->balance++;
}

// Fine-grained Lock Dequeue
int fg_Dequeue() {
	Node* delNode = fine_car_queue->front;
	fine_car_queue->front = fine_car_queue->front->next;
	fine_car_queue->balance--;
	
	int car_num = delNode->car_num;
	free(delNode);
	return car_num;
}

// Without Lock Produce
void* Produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(car_queue->produce_number < total_car) {
		
		int tq = *time_quantum;		
		sleep(tq/100);		// time it takes to release the vehicle
		
		while(car_queue->balance == MAX_SIZE);
			
		Enqueue(car_queue, (car_queue->produce_number % CONSUMER_SIZE) +1);
		car_queue->produce_number++;
	}
}

// Without Lock Consume
void* Consume(void *arg) {
	int* consumer_num = (int*) arg;
	while(1) {
		if(car_queue->produce_number == total_car && isEmpty(car_queue)) break;
			
		while(car_queue->produce_number < total_car && car_queue->balance == 0);	
			
		// Dequeue only possible when the consumer and vehicle number match
		if(!isEmpty(car_queue) && *consumer_num == car_queue->front->car_num) {
			int my_car = Dequeue(car_queue);
		}
	}
}

/// Coarse-grained Lock Produce
void* cg_Produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(coarse_car_queue->produce_number < total_car) {
		pthread_mutex_lock(&coarse_car_queue->mutex);
		
		int tq = *time_quantum;		
		sleep(tq/100);		// time it takes to release the vehicle
		
		while(coarse_car_queue->balance == MAX_SIZE)
			pthread_cond_wait(&cg_empty, &coarse_car_queue->mutex);
		
		if(coarse_car_queue->produce_number < total_car) {
			Enqueue(coarse_car_queue, (coarse_car_queue->produce_number % CONSUMER_SIZE) +1);
			coarse_car_queue->produce_number++;
			pthread_cond_signal(&cg_fill);
		}
		else
			pthread_cond_broadcast(&cg_fill);	// means producer released all vehicles
		pthread_mutex_unlock(&coarse_car_queue->mutex);
	}
}

// Coarse-grained Lock Consume
void* cg_Consume(void *arg) {
	int* consumer_num = (int*) arg;
	
	while(1) {
		if(coarse_car_queue->produce_number == total_car && isEmpty(coarse_car_queue)) break;	
		pthread_mutex_lock(&coarse_car_queue->mutex);
		
		while(coarse_car_queue->produce_number < total_car && coarse_car_queue->balance == 0)
			pthread_cond_wait(&cg_fill, &coarse_car_queue->mutex);
			
		// Dequeue only possible when the consumer and vehicle number match
		if(!isEmpty(coarse_car_queue) && *consumer_num == coarse_car_queue->front->car_num) { 
			int my_car = Dequeue(coarse_car_queue);
			pthread_cond_signal(&cg_empty);
		}
		pthread_mutex_unlock(&coarse_car_queue->mutex);
	}
}

// Fine-grained Lock Produce
void* fg_Produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(fine_car_queue->produce_number < total_car) {
		int tq = *time_quantum;		
		sleep(tq/100);		// time it takes to release the vehicle
		
		pthread_mutex_lock(&fine_car_queue->mutex);
		while(fine_car_queue->balance == MAX_SIZE) 
			pthread_cond_wait(&fg_empty, &fine_car_queue->mutex);	
		pthread_mutex_unlock(&fine_car_queue->mutex);			
		
		pthread_mutex_lock(&fine_car_queue->tailLock);
		if(fine_car_queue->produce_number < total_car) {
			fg_Enqueue((fine_car_queue->produce_number % CONSUMER_SIZE) +1);
			fine_car_queue->produce_number++;
		}
		pthread_mutex_unlock(&fine_car_queue->tailLock);
		
		pthread_mutex_lock(&fine_car_queue->mutex);
		if(fine_car_queue->produce_number < total_car)
			pthread_cond_signal(&fg_fill);
		else 
			pthread_cond_broadcast(&fg_fill);	// means producer released all vehicles
		pthread_mutex_unlock(&fine_car_queue->mutex);	
	}
}

// Fine-grained Lock Consume
void* fg_Consume(void* arg) {
	int* consumer_num = (int*) arg;
	
	while(1) {
		if(fine_car_queue->produce_number == total_car && isEmpty(fine_car_queue)) break;	

		pthread_mutex_lock(&fine_car_queue->mutex);
		while(fine_car_queue->produce_number < total_car && fine_car_queue->balance == 0)
			pthread_cond_wait(&fg_fill, &fine_car_queue->mutex);
		pthread_mutex_unlock(&fine_car_queue->mutex);
		
		// Dequeue only possible when the consumer and vehicle number match
		pthread_mutex_lock(&fine_car_queue->headLock);
		if(!isEmpty(fine_car_queue) && *consumer_num == fine_car_queue->front->car_num) { 
			int my_car = fg_Dequeue();
			pthread_mutex_lock(&fine_car_queue->mutex);
			pthread_cond_signal(&fg_empty);
			pthread_mutex_unlock(&fine_car_queue->mutex);
		}		
		pthread_mutex_unlock(&fine_car_queue->headLock);
	}
}

void lab2_sync_usage(char *cmd) {
	printf("\n Usage for %s : \n",cmd);
	printf("    -c: Total number of vehicles produced, must be bigger than 0 ( e.g. 100 )\n");
	printf("    -q: RoundRobin Time Quantum, must be bigger than 0 ( e.g. 1, 4 ) \n");
}

void lab2_sync_example(char *cmd) {
	printf("\n Example : \n");
	printf("    #sudo %s -c=100 -q=1 \n", cmd);
	printf("    #sudo %s -c=10000 -q=4 \n", cmd);
}

int main(int argc, char* argv[]) {
	char op;
	int n; char junk;
	int time_quantum = 0;

	if (argc <= 1) {
		lab2_sync_usage(argv[0]);
		lab2_sync_example(argv[0]);
		exit(0);
	}

	for (int i = 1; i < argc; i++) {
		if (sscanf(argv[i], "-c=%d%c", &n, &junk) == 1) {
			total_car = n;
		}
		else if (sscanf(argv[i], "-q=%d%c", &n, &junk) == 1) {
			time_quantum = n;
		}
		else {
			lab2_sync_usage(argv[0]);
			lab2_sync_example(argv[0]);
			exit(0);
		}
	}
/*	
	// Segmentation fault
	/////////////// Without lock ///////////////
	struct timeval start, end, gap;
	int status = 0;
	pthread_t Producer;
	pthread_t Consumer[CONSUMER_SIZE];
	double result_T;
	
	Init();
	gettimeofday(&start, NULL);
	
	// create producer thread
	status = pthread_create(&Producer, NULL, Produce, &time_quantum);
	assert(status == 0);
	
	// create consumer thread
	for(int i=0; i<CONSUMER_SIZE; i++) {
		int* consumer_num = (int*)malloc(sizeof(int));
		*consumer_num = i+1;
		status = pthread_create(&Consumer[i], NULL, Consume, consumer_num);
		assert(status == 0);
	}
	
	pthread_join(Producer, NULL);
	for(int i=0; i<CONSUMER_SIZE; i++)
		pthread_join(Consumer[i], NULL);
		
	gettimeofday(&end, NULL);
	result_T = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0);
	
	gap.tv_sec = end.tv_sec - start.tv_sec;
	gap.tv_usec = end.tv_usec - start.tv_usec;
	if(gap.tv_usec < 0) {
		gap.tv_sec -= 1; 
		gap.tv_usec += 1000000;
	}
	
	printf("==== Vehicle Production Problem ====\n");
	printf("(1) No Lock Experiment\n");
	printf("Experiment Info\n");
	printf("\tTotal Produce Number = %d\n", car_queue->produce_number);
	printf("\tFinal Balance Value = %d\n", car_queue->balance);
	printf("\tExecution time = %f -> %ldsec:%ldusec\n", result_T, gap.tv_sec, gap.tv_usec);
*/
	/////////////// Coarse-grained lock ///////////////
	struct timeval cg_start, cg_end, cg_gap;
	int cg_status = 0;
	pthread_t cg_Producer[PRODUCER_SIZE];
	pthread_t cg_Consumer[CONSUMER_SIZE];
	double result_cgT;
		
	cg_Init();	// initialize queue, condition variable, lock variable
	gettimeofday(&cg_start, NULL);
	
	// create producer thread
	for(int i=0; i<PRODUCER_SIZE; i++) {
		cg_status = pthread_create(&cg_Producer[i], NULL, cg_Produce, &time_quantum);
		assert(cg_status == 0);
	}

	// create consumer thread
	for(int i=0; i<CONSUMER_SIZE; i++) {
		int* consumer_num = (int*)malloc(sizeof(int));
		*consumer_num = i+1;
		cg_status = pthread_create(&cg_Consumer[i], NULL, cg_Consume, consumer_num);
		assert(cg_status == 0);
	}
	
	for(int i=0; i<PRODUCER_SIZE; i++)
		pthread_join(cg_Producer[i], NULL);
	for(int i=0; i<CONSUMER_SIZE; i++)
		pthread_join(cg_Consumer[i], NULL);
	
	gettimeofday(&cg_end, NULL);
	result_cgT = (cg_end.tv_sec - cg_start.tv_sec) + ((cg_end.tv_usec - cg_start.tv_usec)/1000000.0);
	
	cg_gap.tv_sec = cg_end.tv_sec - cg_start.tv_sec;
	cg_gap.tv_usec = cg_end.tv_usec - cg_start.tv_usec;
	if(cg_gap.tv_usec < 0) {
		cg_gap.tv_sec -= 1; 
		cg_gap.tv_usec += 1000000;
	}

	printf("==== Vehicle Production Problem ====\n");
	printf("(2) Coarse-grained Lock Experiment\n");
	printf("Experiment Info\n");
	printf("\tTotal Produce Number = %d\n", coarse_car_queue->produce_number);
	printf("\tFinal Balance Value = %d\n", coarse_car_queue->balance);
	printf("\tExecution time = %f -> %ldsec:%ldusec\n", result_cgT, cg_gap.tv_sec, cg_gap.tv_usec);
	
	/////////////// Fine-grained lock ///////////////
	struct timeval fg_start, fg_end, fg_gap;
	int fg_status = 0;
	pthread_t fg_Producer[PRODUCER_SIZE];
	pthread_t fg_Consumer[CONSUMER_SIZE];
	double result_fgT;
	
	fg_Init();
	gettimeofday(&fg_start, NULL);
	
	// create producer thread
	for(int i=0; i<PRODUCER_SIZE; i++) {
		fg_status = pthread_create(&fg_Producer[i], NULL,fg_Produce, &time_quantum);
		assert(fg_status == 0);
	}
	
	// create consumer thread
	for(int i=0; i<CONSUMER_SIZE; i++) {
		int* consumer_num = (int*)malloc(sizeof(int));
		*consumer_num = i+1;
		fg_status = pthread_create(&fg_Consumer[i], NULL, fg_Consume, consumer_num);
		assert(fg_status == 0);
	}
	
	for(int i=0; i<PRODUCER_SIZE; i++)		
		pthread_join(fg_Producer[i], NULL);
	for(int i=0; i<CONSUMER_SIZE; i++)
		pthread_join(fg_Consumer[i], NULL);
	
	gettimeofday(&fg_end, NULL);
	result_fgT = (fg_end.tv_sec - fg_start.tv_sec) + ((fg_end.tv_usec - fg_start.tv_usec)/1000000.0);

	fg_gap.tv_sec = fg_end.tv_sec - fg_start.tv_sec;
	fg_gap.tv_usec = fg_end.tv_usec - fg_start.tv_usec;
	if(fg_gap.tv_usec < 0) {
		fg_gap.tv_sec -= 1; 
		fg_gap.tv_usec += 1000000;
	}

	printf("==== Vehicle Production Problem ====\n");
	printf("(3) Fine-grained Lock Experiment\n");
	printf("Experiment Info\n");
	printf("\tTotal Produce Number = %d\n", fine_car_queue->produce_number);
	printf("\tFinal Balance Value = %d\n", fine_car_queue->balance);
	printf("\tExecution time = %f -> %ldsec:%ldusec\n", result_fgT, fg_gap.tv_sec, fg_gap.tv_usec);

	return 0;
}
