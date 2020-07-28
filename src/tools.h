#include <stdio.h>
#include <stdlib.h>

typedef struct Node Node;
typedef struct Process Process;
typedef struct Queue Queue;

struct Process
{
    int class;
    int pid;
    int priority;
    int ptime;
};

Process* createPCB(int, int, int, int);

struct Node
{
    Node* pre;
    Node* next;
    Process* process;
};

Node* createNode(Process*);

struct Queue
{
    Node* front;
    Node* rear;
    int size;
};

Queue* createQueue();
int isEmpty(Queue*);
void push(Queue*, Node*);
Node* pop(Queue*);
void removeNode(Queue*, Node*);
void printQueue(Queue*, char*);

Process* createPCB(int class, int pid, int prior, int ptime) {
    Process* process = (Process*)malloc(sizeof(Process));

    process->class = class;
    process->pid = pid;
    process->priority = prior;
    process->ptime = ptime;

    return process;
}

Node* createNode(Process* pcb) {
    Node* node = (Node*)malloc(sizeof(Node));

    node->pre = NULL;
    node->next = NULL;
    node->process = pcb;

    return node;
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));

    q->front = NULL;
    q->rear = NULL;
    q->size = 0;

    return q;
}

int isEmpty(Queue* q) {
    if (q->size == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

void push(Queue* q, Node* node) {
    if (node != NULL) {
        if (isEmpty(q)) {
            q->front = node;
        }
        else {
            q->rear->next = node;
            node->pre = q->rear;
        }
        q->rear = node;
        q->size++;
    }
}

Node* pop(Queue* q){
    Node* node = NULL;
    if(!isEmpty(q)){
        node = q->front;
        if(q->size > 1){
            q->front = node->next;
            q->front->pre = NULL;
        }
        else{
            q->front = NULL;
            q->rear = NULL;
        }
        node->pre = NULL;
        node->next = NULL;
        q->size--;
    }

    return node;
}

void removeNode(Queue* q, Node* node) {

    if(!isEmpty(q)){
        if(q->size > 1){
            if (node->next == NULL) {
                q->rear = node->pre;
                node->pre->next = NULL;
            }
            else if (node->pre == NULL) {
                q->front = node->next;
                node->next->pre = NULL;
            }
            else {
                node->pre->next = node->next;
                node->next->pre = node->pre;
            }
        }
        else{
            q->front = NULL;
            q->rear = NULL;
        }
        node->pre = NULL;
        node->next = NULL;

        q->size--;
    }
}

void printQueue(Queue* q, char* queueName){
    Node* node = q->front;
    
    printf("============ %s ============\n",queueName);
    printf("class  pid  priority  ptime\n");
	while(node != NULL){ 
		printf("  %d    %2d     %2d       %2d\n",node->process->class,node->process->pid,node->process->priority, node->process->ptime);
		node = node->next;
	}
}



