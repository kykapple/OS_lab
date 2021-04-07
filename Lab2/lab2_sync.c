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
 
CQ* car_queue = NULL;	// producer and consumers share the queue
int total_car;

void print() {
	Node* cur = car_queue->front;
	while(cur) {
		printf("%d ", cur->car_num);
		cur = cur->next;
	}
	printf("\n");
}

void init() {
	car_queue = (CQ*) malloc(sizeof(CQ));	// initialize car_queue
	car_queue->balance = 0;
	car_queue->total_produce_number = 0;
	car_queue->front = NULL;
	car_queue->rear = NULL;
	
	pthread_cond_init(&fill, NULL);
	pthread_cond_init(&empty, NULL);
	
	pthread_mutex_init(&mutex, NULL);
}

int isEmpty() {
	return car_queue->front == NULL ? 1 : 0;	// if queue is empty return 1
}

void Enqueue(int car_num) {
	Node* newNode = (Node*) malloc(sizeof(Node));
	assert(newNode != NULL);
	
	newNode->car_num = car_num;
	newNode->next = NULL;
	
	if(car_queue->front == NULL)
		car_queue->front = newNode;
	else
		car_queue->rear->next = newNode;
	
	car_queue->rear = newNode;
	car_queue->balance++;
}

int Dequeue() {
	Node* delNode = car_queue->front;
	car_queue->front = car_queue->front->next;
	
	car_queue->balance--;
	int car_num = delNode->car_num;
	free(delNode);

	return car_num;
}

void* produce(void* arg) {
	int* time_quantum = (int*) arg;

	while(car_queue->total_produce_number < total_car) {
		pthread_mutex_lock(&mutex);
		
		int tq = *time_quantum;
		//while(tq--);		// time it takes to release the vehicle
		
		sleep(tq/10);
		
		while(car_queue->balance == MAX_SIZE)
			pthread_cond_wait(&empty, &mutex);
		Enqueue((car_queue->total_produce_number % 5) +1);
		car_queue->total_produce_number++;
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
/*		printf("producer balance = %d total car = %d\n", car_queue->balance, total_car);
		printf("producer thread unlock:");
		print();
*/	}
	
	// means producer released all vehicles
	pthread_cond_broadcast(&fill);
}

void* consume(void *arg) {
	int* consumer_num = (int*) arg;
	while(1) {
		if(car_queue->total_produce_number == total_car && car_queue->balance == 0) break;
		
		pthread_mutex_lock(&mutex);
		
		while(car_queue->total_produce_number < total_car && car_queue->balance == 0)
			pthread_cond_wait(&fill, &mutex);
			
		// Dequeue only possible when the consumer and vehicle number match
		if(!isEmpty() && *consumer_num == car_queue->front->car_num) { 
			int my_car = Dequeue();
			pthread_cond_signal(&empty);
//			printf("%d : my car : %d\n", *consumer_num, my_car);
		}
	
//		printf("consumer balance = %d\n", car_queue->balance);
		pthread_mutex_unlock(&mutex);
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
	
	struct timeval start, end, gap;
	
	int status = 0;
	pthread_t producer;
	pthread_t consumer[CONSUMER_SIZE];

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
	
	init();	// initialize queue, condition variable, lock variable
	
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
	gap.tv_sec = end.tv_sec - start.tv_sec;
	gap.tv_usec = end.tv_usec - start.tv_usec;
	if(gap.tv_usec < 0) {
		gap.tv_sec -= 1; 
		gap.tv_usec += 1000000;
	}
	
	printf("Total Produce Number = %d\n", car_queue->total_produce_number);
	printf("Final Balance Value = %d\n", car_queue->balance);
	printf("Execution time = %ldsec:%ldusec\n", gap.tv_sec, gap.tv_usec);
	
	
	return 0;
}
