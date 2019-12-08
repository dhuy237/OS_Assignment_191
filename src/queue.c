#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	/* TODO: put a new process to queue [q] */
	if(q->size==MAX_QUEUE_SIZE)
	return;
	if (empty(q))
	{
		q->proc[0]=proc;
		q->size++;
		return;
	}
	else {
		int i;
		for( i=q->size-1;i>=0 && q->proc[i]->priority >  proc->priority;i--)
		{
			q->proc[i+1]=q->proc[i];
		}
		q->proc[i+1]=proc;
		q->size++;
		return;
	}

}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	if(q->size!=0 )
	{
		q->size--;
		struct pcb_t *temp = q->proc[q->size];
		q->proc[q->size]=NULL;
			return temp;
	}
	return NULL;
	
}

