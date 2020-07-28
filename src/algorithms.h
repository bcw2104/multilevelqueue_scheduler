#include "tools.h"

Node* sjf_algorithms(Queue*);
Node* rr_algorithms(Queue*, int);
Node* priority_algorithms(Queue*);

Node* sjf_algorithms(Queue* q) {
	if (!isEmpty(q)) {
		Node* temp = q->front->next;
		Node* min = q->front;
		while (temp != NULL) {
			if (min->process->ptime > temp->process->ptime) {
				min = temp;
			}
			temp = temp->next;
		}
		removeNode(q, min);
		return min;
	}
	else {
		return NULL;
	}
	
}

Node* rr_algorithms(Queue* q, int quantum) {
	Node* node;
	Process* p;
	if ((node = pop(q)) != NULL) {
		if (node->process->ptime > quantum) {
			push(q, createNode(createPCB(node->process->class, node->process->pid, node->process->priority, node->process->ptime-quantum)));

			node->process->ptime = quantum;
		}
		return node;
	}
	else {
		return NULL;
	}
}
Node* priority_algorithms(Queue* q) {
	if (!isEmpty(q)) {
		Node* temp = q->front->next;
		Node* min = q->front;

		while (temp != NULL) {
			if (min->process->priority > temp->process->priority) {
				min = temp;
			}
			temp = temp->next;
		}
		removeNode(q, min);

		return min;
	}
	else {
		return NULL;
	}
}