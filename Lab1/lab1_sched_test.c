/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32170260
*	    Student name : Kweon Yeong Kee
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm test code.
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

int main(int argc, char *argv[]){
	int job_cnt;

	printf("------------scheduling simulator------------\n");
	printf("		1. FIFO		      \n");
	printf("		2. SJF			      \n");
	printf("		3. RR			      \n");
	printf("		4. MLFQ		      \n");
	printf("		5. Lottery		      \n");
	printf("          made by Kweon Yeong Kee           \n");
	printf("--------------------------------------------\n");
	
	printf("Enter the number of jobs: ");
	scanf("%d", &job_cnt);
	
	printf("FIFO\n");
//	FIFO(job_cnt);
	
	printf("SJF\n");
//	SJF(job_cnt);
	
	printf("RR with time quantum 1\n");
//	RR(job_cnt);
	
	printf("RR with time quantum 4\n");
//	RR(job_cnt);
	
	printf("MLFQ with time quantum 1\n");
//	MLFQ(job_cnt, 1);
	
	printf("MLFQ with time quantum 2^i\n");
//	MLFQ(job_cnt, 4);
	
	printf("Lottery\n");
	Lottery(job_cnt);
    return 0;
}
