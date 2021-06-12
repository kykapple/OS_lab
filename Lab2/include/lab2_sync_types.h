/*
*   Lab2 (Vehicle Production Problem)
*
*   lab1_sync_types.h :
*       - lab2 header file.
*       - must contains Vehicle Production Problem's declations.
*
*/

#ifndef _LAB2_HEADER_H
#define _LAB2_HEADER_H

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#define MAX_SIZE 10
#define PRODUCER_SIZE 5
#define CONSUMER_SIZE 5

typedef struct Node {
	int car_num;
	struct Node* next;
}Node;

typedef struct car_queue {
	int balance;
	int produce_number;
	Node* front;
	Node* rear;
	
	pthread_mutex_t mutex;
	/*
	* find-grained
	*/
	pthread_mutex_t headLock;
	pthread_mutex_t tailLock;
}CQ;

pthread_cond_t cg_fill, cg_empty;
pthread_cond_t fg_fill, fg_empty;
 
void print(CQ* current_queue);
int isEmpty(CQ* current_queue);

void Init();
void cg_Init();
void fg_Init();

void Enqueue();
int Dequeue(CQ* current_queue);
void fg_Enqueue();
int fg_Dequeue();

void* Produce(void* arg);
void* Consume(void* arg);
void* cg_Produce(void* arg);
void* cg_Consume(void* arg);
void* fg_Produce(void* arg);
void* fg_Consume(void* arg);

#endif /* LAB2_HEADER_H*/

