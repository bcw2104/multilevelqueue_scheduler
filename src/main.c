#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "algorithms.h"
#define Queue_Count 3
#define Thread_Count 4

sem_t empty,full;
int flag[Queue_Count] = {1,1,1};
Queue* ready;
FILE* output;

Queue** read_file(char*);
void request_Node(int);
void* thread_main(void*);
void* thread_sub1(void*);
void* thread_sub2(void*);
void* thread_sub3(void*);

int main(int argc, char* argv[])
{
	pthread_t thread[Thread_Count];
	pthread_attr_t attr;

	sem_init(&empty, 0, 1);
	sem_init(&full, 0, 0);

	if (argc == 1) {
		printf("No Input!\n");
		return -1;
	}
	else {
		Queue** q_arr = read_file(argv[1]);
		ready = createQueue();
		output = fopen("output.txt", "w");

		if (q_arr != NULL) {
			pthread_attr_init(&attr);
			pthread_create(&thread[0], &attr, thread_main, NULL);
			pthread_create(&thread[1], &attr, thread_sub1, (void*)q_arr[0]);
			pthread_create(&thread[2], &attr, thread_sub2, (void*)q_arr[1]);
			pthread_create(&thread[3], &attr, thread_sub3, (void*)q_arr[2]);

			pthread_join(thread[0], NULL);
			pthread_join(thread[1], NULL);
			pthread_join(thread[2], NULL);
			pthread_join(thread[3], NULL);

			printf("Scheduler Finished\n");
			free(ready);
			free(q_arr);
			fclose(output);
			return 0;
		}
		else {
			printf("No File!\n");
			return -1;
		}
	}
}

Queue** read_file(char* f) {
	FILE* input = fopen(f, "r");
	Queue** q_arr = NULL;

	if (input != NULL) {
		q_arr = (Queue**)malloc(sizeof(Queue*) * Queue_Count);
		for (int i = 0; i < Queue_Count; i++) {
			q_arr[i] = createQueue();
		}

		int class, pid, priority, ptime;

		while (!feof(input) && fscanf(input, "%d %d %d %d", &class, &pid, &priority, &ptime)!= -1) {
			Process* p = createPCB(class, pid, priority, ptime);
			push(q_arr[p->class - 1], createNode(p));					
		}

		fclose(input);
	}
	else {
		printf("No File!");
	}

	return q_arr;
}

void request_Node(int class){
	if(flag[class] != -1){
		flag[class] = 1;
		sem_post(&empty);
		sem_wait(&full);
		while(flag[class] > 0);
		if(flag[class] == -1){
			for(int i=0; i<Queue_Count; i++){
				if(flag[i] != -1){	
					flag[i] = 1;
					sem_post(&empty);
					sem_wait(&full);
					while(flag[i] > 0);
					if(flag[i] != -1){
						break;
					}				
				}
			}				
		}	
	}
	else{
		for(int i=0; i<Queue_Count; i++){
			if(flag[i] != -1){	
				flag[i] = 1;
				sem_post(&empty);
				sem_wait(&full);
				while(flag[i] > 0);
				if(flag[i] != -1){
					break;
				}
			}				
		}
	}
}

void* thread_main(void* param) {
	printf("thread[0] created\n");
	Node* node;
	Process* p;
	int quantum[3] = {7,5,3};
	sem_wait(&full);
	printQueue(ready,"ready");
	while ((node = pop(ready)) != NULL){
		p = node->process;
		printf("scheduler : ");
		if(p->ptime > quantum[p->class-1]){
			for (int i = 0; i < quantum[p->class-1]; i++) {
				printf("%d ",p->pid);
				fprintf(output, "%d ", p->pid);
			}
			p->ptime =  p->ptime - quantum[p->class-1];
			push(ready,node);
		}
		else{
			while(p->ptime > 0){
				printf("%d ",p->pid);
				fprintf(output, "%d ", p->pid);
				p->ptime --;
			}
			request_Node(p->class-1);

			free(node);
			free(p);
		}
		printf("\n");
		fprintf(output, "\n");	

		printQueue(ready,"ready");
	}
	printf("\nthread[0] terminate\n");
	pthread_exit(0);
}

//Priority
void* thread_sub1(void* param) {
	printf("thread[1] created\n");
	Node* node;
	Queue* q = (Queue*)param;
	int loop = 1;
	
	while(loop){
		while(!flag[0]);
		sem_wait(&empty);
		if ((node = priority_algorithms(q)) != NULL) {
			push(ready, node);
			flag[0] = 0;
		}
		else{
			flag[0] = -1;
			loop = 0;
		}
		if(flag[1] > 0 || flag[2] > 0){
			sem_post(&empty);
		}
		else{
			sem_post(&full);
		}
	}
	printf("\nthread[1] terminate\n");
	free(q);
	
	pthread_exit(0);
}

//RoundRobin
void* thread_sub2(void* param) {
	printf("thread[2] created\n");
	Node* node;
	Queue* q = (Queue*)param;
	int loop = 1;
	
	while(loop){
		while(!flag[1]);
		sem_wait(&empty);
		if ((node = rr_algorithms(q,5)) != NULL) {
			push(ready, node);
			flag[1] = 0;
		}
		else{
			flag[1] = -1;
			loop = 0;
		}
		if(flag[0] > 0 || flag[2] > 0){
			sem_post(&empty);
		}
		else{
			sem_post(&full);
		}
	}
	printf("\nthread[2] terminate\n");
	free(q);
	
	pthread_exit(0);
}

//SJF
void* thread_sub3(void* param) {
	printf("thread[3] created\n");
	Node* node;
	Queue* q = (Queue*)param;
	int loop = 1;
	
	while(loop){
		while(!flag[2]);
		sem_wait(&empty);
		if ((node = sjf_algorithms(q)) != NULL) {
			push(ready, node);
			flag[2] = 0;
		}
		else{
			flag[2] = -1;
			loop = 0;
		}
		if(flag[0] > 0 || flag[1] > 0){
			sem_post(&empty);
		}
		else{
			sem_post(&full);
		}
	}
	printf("\nthread[3] terminate\n");
	free(q);
	
	pthread_exit(0);
}
