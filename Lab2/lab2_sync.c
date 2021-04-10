/*
*   DKU Operating System Lab
*           Lab2 (Vehicle production Problem)
*           Student id : 32170260
*           Student name : Kweon Yeong Kee
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
 
CQ* coarse_car_queue = NULL;	// producer and consumers share the queue
CQ* fine_car_queue = NULL;
int total_car;
/*
void print(CQ* car_queue) {
	Node* cur = car_queue->front;
	while(cur) {
		printf("%d ", cur->car_num);
		cur = cur->next;
	}
	printf("\n");
}
*/
void coarse_init() {
	coarse_car_queue = (CQ*) malloc(sizeof(CQ));	// initialize car_queue
	coarse_car_queue->balance = 0;
	coarse_car_queue->produce_number = 0;
	coarse_car_queue->front = NULL;
	coarse_car_queue->rear = NULL;
	
	pthread_cond_init(&cg_fill, NULL);
	pthread_cond_init(&cg_empty, NULL);
	
	pthread_mutex_init(&coarse_car_queue->mutex, NULL);
}

void fine_init() {
	fine_car_queue = (CQ*) malloc(sizeof(CQ));	// initialize car_queue
	fine_car_queue->balance = 0;
	fine_car_queue->produce_number = 0;
	fine_car_queue->front = NULL;
	fine_car_queue->rear = NULL;
	
	pthread_cond_init(&fg_fill, NULL);
	pthread_cond_init(&fg_empty, NULL);
	
	pthread_mutex_init(&fine_car_queue->mutex, NULL);
	/*
	* test fine-grained
	*/
	pthread_mutex_init(&fine_car_queue->headLock, NULL);
	pthread_mutex_init(&fine_car_queue->tailLock, NULL);
}

int isEmpty(CQ* car_queue) {
	return car_queue->front == NULL ? 1 : 0;	// if queue is empty return 1
}

void Enqueue(int car_num) {
	Node* newNode = (Node*) malloc(sizeof(Node));
	assert(newNode != NULL);
	
	newNode->car_num = car_num;
	newNode->next = NULL;
	
	if(coarse_car_queue->front == NULL)
		coarse_car_queue->front = newNode;
	else
		coarse_car_queue->rear->next = newNode;
	
	coarse_car_queue->rear = newNode;
	coarse_car_queue->balance++;
}

int Dequeue() {
	Node* delNode = coarse_car_queue->front;
	coarse_car_queue->front = coarse_car_queue->front->next;
	
	coarse_car_queue->balance--;
	int car_num = delNode->car_num;
	free(delNode);

	return car_num;
}

void* produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(coarse_car_queue->produce_number < total_car) {
		pthread_mutex_lock(&coarse_car_queue->mutex);
		
		int tq = *time_quantum;		
		sleep(tq/10);		// time it takes to release the vehicle
		
		while(coarse_car_queue->balance == MAX_SIZE)
			pthread_cond_wait(&cg_empty, &coarse_car_queue->mutex);
		Enqueue((coarse_car_queue->produce_number % CONSUMER_SIZE) +1);
		coarse_car_queue->produce_number++;
		
		if(coarse_car_queue->produce_number < total_car)
			pthread_cond_signal(&cg_fill);
		else
			pthread_cond_broadcast(&cg_fill);	// means producer released all vehicles
		pthread_mutex_unlock(&coarse_car_queue->mutex);
	}
}

void* consume(void *arg) {
	int* consumer_num = (int*) arg;
	
	while(1) {
		if(coarse_car_queue->produce_number == total_car && coarse_car_queue->balance == 0) break;
		
		pthread_mutex_lock(&coarse_car_queue->mutex);
		
		while(coarse_car_queue->produce_number < total_car && coarse_car_queue->balance == 0)
			pthread_cond_wait(&cg_fill, &coarse_car_queue->mutex);
			
		// Dequeue only possible when the consumer and vehicle number match
		if(!isEmpty(coarse_car_queue) && *consumer_num == coarse_car_queue->front->car_num) { 
			int my_car = Dequeue();
			pthread_cond_signal(&cg_empty);
		}
	
		pthread_mutex_unlock(&coarse_car_queue->mutex);
	}
}

void fine_Enqueue(int car_num) {
	Node* newNode = (Node*) malloc(sizeof(Node));
	assert(newNode != NULL);
	
	newNode->car_num = car_num;
	newNode->next = NULL;
	
	pthread_mutex_lock(&fine_car_queue->tailLock);
	if(fine_car_queue->front == NULL)
		fine_car_queue->front = newNode;
	else
		fine_car_queue->rear->next = newNode;
	
	fine_car_queue->rear = newNode;
	fine_car_queue->balance++;
	pthread_mutex_unlock(&fine_car_queue->tailLock);
}


int fine_Dequeue() {
	Node* delNode = fine_car_queue->front;
	fine_car_queue->front = fine_car_queue->front->next;
	fine_car_queue->balance--;
	
	int car_num = delNode->car_num;
	free(delNode);
	return car_num;
}

void* fine_produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(fine_car_queue->produce_number < total_car) {
		int tq = *time_quantum;		
		sleep(tq/100);		// time it takes to release the vehicle
//		sleep(1);
		pthread_mutex_lock(&fine_car_queue->mutex);
		
		while(fine_car_queue->balance == MAX_SIZE) {
			pthread_cond_broadcast(&fg_fill);
			pthread_cond_wait(&fg_empty, &fine_car_queue->mutex);		
		}
		
		fine_Enqueue((fine_car_queue->produce_number % CONSUMER_SIZE) +1);
		fine_car_queue->produce_number++;
		
		if(fine_car_queue->produce_number < total_car)
			pthread_cond_signal(&fg_fill);
		else {
			pthread_cond_broadcast(&fg_fill);	// means producer released all vehicles
					
		}
		pthread_mutex_unlock(&fine_car_queue->mutex);	
	}
}

void* fine_consume(void* arg) {
	int* consumer_num = (int*) arg;
	
	while(1) {
		if(fine_car_queue->produce_number == total_car && fine_car_queue->balance == 0) break;
	//	sleep(1);
		pthread_mutex_lock(&fine_car_queue->mutex);
		
		while(fine_car_queue->produce_number < total_car && fine_car_queue->balance == 0) {
			pthread_cond_wait(&fg_fill, &fine_car_queue->mutex);
		}
		
		pthread_mutex_unlock(&fine_car_queue->mutex);
		
		while(fine_car_queue->produce_number < total_car) {
			pthread_mutex_lock(&fine_car_queue->headLock);
			
			if(!isEmpty(fine_car_queue) && fine_car_queue->front->car_num == *consumer_num) {
				pthread_mutex_unlock(&fine_car_queue->headLock);
				break;
			}	
			
			pthread_mutex_unlock(&fine_car_queue->headLock);
			
			pthread_mutex_lock(&fine_car_queue->mutex);
			
			while (fine_car_queue->produce_number < total_car && !isEmpty(fine_car_queue) && fine_car_queue->front->car_num != *consumer_num) {
				pthread_cond_wait(&fg_fill, &fine_car_queue->mutex);
			}
			pthread_mutex_unlock(&fine_car_queue->mutex);
		}
		
		// Dequeue only possible when the consumer and vehicle number match
		pthread_mutex_lock(&fine_car_queue->headLock);
		if(!isEmpty(fine_car_queue) && *consumer_num == fine_car_queue->front->car_num) { 
			int my_car = fine_Dequeue();
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
		
	/////////////// coarse-grained-lock ver ////////////////////////
/*	double cg_avg = 0;
	printf("Coarse-grained Lock\n");
	for(int i=0; i<10; i++) {
	struct timeval start, end, gap;
	int status = 0;
	pthread_t producer;
	pthread_t consumer[CONSUMER_SIZE];
	double result_cgT;
		
	coarse_init();	// initialize queue, condition variable, lock variable
	gettimeofday(&start, NULL);
	
	// create producer thread
	status = pthread_create(&producer, NULL, produce, &time_quantum);
	assert(status == 0);

	// create consumer thread
	for(int i=0; i<CONSUMER_SIZE; i++) {
		int* consumer_num = (int*)malloc(sizeof(int));
		*consumer_num = i+1;
		status = pthread_create(&consumer[i], NULL, consume, consumer_num);
		assert(status == 0);
	}
	
	pthread_join(producer, NULL);
	for(int i=0; i<CONSUMER_SIZE; i++)
		pthread_join(consumer[i], NULL);
	
	gettimeofday(&end, NULL);
	result_cgT = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0);

	printf("\tExecution time = %f\n", result_cgT);
	cg_avg += result_cgT;
	}
	cg_avg /= 10.0;
	printf("\tAverage Execution Time = %f\n", cg_avg);
	
/*	gap.tv_sec = end.tv_sec - start.tv_sec;
	gap.tv_usec = end.tv_usec - start.tv_usec;
	if(gap.tv_usec < 0) {
		gap.tv_sec -= 1; 
		gap.tv_usec += 1000000;
	}
	
*/	/////////////////////////////////////////////////////////////////////
	
	/////////////// fine-grained-lock ver ////////////////////////	
	double fg_avg =0;
	printf("Fine-grained Lock\n");
	for(int i=0; i<10; i++) {
	struct timeval s, e, g;
	int status2 = 0;
	pthread_t producer1;
	pthread_t consumer1[CONSUMER_SIZE];
	double result_fgT;
	
	fine_init();
	gettimeofday(&s, NULL);
	
	// create producer thread
	status2 = pthread_create(&producer1, NULL, fine_produce, &time_quantum);
	assert(status2 == 0);
	
	// create consumer thread
	for(int i=0; i<CONSUMER_SIZE; i++) {
		int* consumer_num = (int*)malloc(sizeof(int));
		*consumer_num = i+1;
		status2 = pthread_create(&consumer1[i], NULL, fine_consume, consumer_num);
		assert(status2 == 0);
	}
	
	pthread_join(producer1, NULL);
	for(int i=0; i<CONSUMER_SIZE; i++)
		pthread_join(consumer1[i], NULL);
	
	gettimeofday(&e, NULL);
	result_fgT = (e.tv_sec - s.tv_sec) + ((e.tv_usec - s.tv_usec)/1000000.0);

	printf("\tExecution time = %f\n", result_fgT);
	fg_avg += result_fgT;
	}
	fg_avg /= 10.0;
	printf("\tAverage Execution Time = %f\n", fg_avg);
/*	g.tv_sec = e.tv_sec - s.tv_sec;
	g.tv_usec = e.tv_usec - s.tv_usec;
	if(g.tv_usec < 0) {
		g.tv_sec -= 1; 
		g.tv_usec += 1000000;
	}
*/	
	/////////////////////////////////////////////////////////////////////
	/*
	printf("(2) Coarse-grained Lock Experiment\n");
	printf("Experiment Info\n");
	printf("\tTotal Produce Number = %d\n", coarse_car_queue->produce_number);
	printf("\tFinal Balance Value = %d\n", coarse_car_queue->balance);
	printf("\tExecution time = %ldsec:%ldusec\n", gap.tv_sec, gap.tv_usec);
	
	printf("(3) Fine-grained Lock Experiment\n");
	printf("Experiment Info\n");
	printf("\tTotal Produce Number = %d\n", fine_car_queue->produce_number);
	printf("\tFinal Balance Value = %d\n", fine_car_queue->balance);
	printf("\tExecution time = %ldsec:%ldusec\n", g.tv_sec, g.tv_usec);
	*/
	/////////////////////////////////////////////////////////////////////
	
	
	
	return 0;
}
