#include <stdio.h>
#include "queue.h"
#include "sched.h"
#include <pthread.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

int queue_empty(void) {
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue]. If ready queue
	 * is empty, push all processes in [run_queue] back to
	 * [ready_queue] and return the highest priority one.
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if(empty(&ready_queue) && !empty(&run_queue)){
printf("Ready queue empty\n");
printf("Run queue-size: %d\n",run_queue.size);
printf("PID,Priority:");
	for(int j=0;j<run_queue.size;j++){
	printf("%d-%d\n ",run_queue.proc[j]->pid, run_queue.proc[j]->priority);
	}

	int i;
		 for(i=0;i<run_queue.size;i++)
		{
			ready_queue.proc[i]=run_queue.proc[i];
			run_queue.proc[i] = NULL;
		}	
		 ready_queue.size = run_queue.size;
		 run_queue.size = 0;
		  printf("Ready queue push\n");
        printf("Ready queue-size: %d\n",ready_queue.size);
	printf("PID,Priority:");
        for(int j=0;j<ready_queue.size;j++){
        printf("%d-%d\n ",ready_queue.proc[j]->pid, ready_queue.proc[j]->priority);
        }


		 proc = dequeue(&ready_queue);
//		 printf("Ready queue push\n");
  //      printf("Ready queue-size: %d\n",ready_queue.size);


	}
	else { printf("Ready queue status\n");
        printf("Ready queue-size: %d\n",ready_queue.size);
	if(ready_queue.size !=0){
        printf("PID,Priority:");
        for(int j=0;j<ready_queue.size;j++){
        printf("%d-%d\n ",ready_queue.proc[j]->pid, ready_queue.proc[j]->priority);
	}}
		proc = dequeue(&ready_queue);	
	}
	pthread_mutex_unlock(&queue_lock);	
	return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}


