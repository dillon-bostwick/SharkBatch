/*
 * Queue.h
 *
 * This Queue implementation was originally written for a previous homework but I made
 * modifications that allows the queue's API to exactly resemble many important functions
 * of the C++ STL queue API. The only addition to STL-like functions is a force_pop that
 * allows me to improve the complexity of the scheduler when the client wants to
 * forcefully kill a premature process
 *
 * It uses a linked list of Nodes containing pointers to jobs. While this might not 
 * optimize complexity, it does not make a significant impact on the overhead of the
 * Scheduler given the queue's purpose.
 */

#ifndef JOBQUEUE_H_
#define JOBQUEUE_H_

#include "Job.h"

class JobQueue {
	public:
         JobQueue();
        ~JobQueue();
        
        //All following functions resemble an STL queue
        void push(Job *j);
        void pop();
        bool empty();
        Job *front();
        int  size();
        
        //Allows removal of a job by PID from anywhere in the queue
        bool force_pop(int pid);

	private:
	//See the .cpp file for diagram of ADT -- next leads to the back, and prev leads to
	//the front
	struct Node {
		Job  *head;
		Node *next;
		Node *prev;
	};
	
	Node *frontPtr;
	Node *backPtr;
	int   sizeCount;
};

#endif /* QUEUE_H_ */
