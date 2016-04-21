/*
 * Queue.h
 *
 * This is the header for a Queue that was originally written for HW3 but I made
 * modifications that allows the queue's API to resemble a C++ STL queue
 * but with a few extra functions that allows me to improve the complexity
 * of the scheduler
 *
 */

#ifndef JOBQUEUE_H_
#define JOBQUEUE_H_

#include "Job.h"

class JobQueue {
public:
        // constructor
        JobQueue();

        // destructor
        ~JobQueue();

        // push a ponter to a job onto at the back of the queue
        void push(Job *j);

        // pop a job from the front of the queue
        void pop();

        // returns true if there are no elements in the
        // queue, false if the queue has elements
        bool empty();
        
        //returns the front but does not delete, just like an STL queue
        Job *front();
        
        int size();
        
        //force remove a job from somewhere within the queue. Worst case O(n) but
        //average case is much better than having to do a dequeue/enqueue
        //cycle from the client's side
        bool force_pop(int pid);
        

private:
	//See .cpp for diagram of ADT. Next leads to
	//the back, where pushs occur, and prev leads
	//to the front, where pops occur.
	struct Node {
		Job  *head;
		Node *next;
		Node *prev;
	};
	
	Node *frontPtr;
	Node *backPtr;
	int sizeCount;
};

#endif /* QUEUE_H_ */
