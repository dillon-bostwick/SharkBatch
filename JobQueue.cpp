/*
 * JobQueue.cpp
 * by Dillon Bostwick
 *
 * Creates a FIFO JobQueue using an
 * implementation of a doubly linked list. 
 * pop occurs at the frontPtr, and push
 * occurs at the backPtr. See illustration:
 *
 * frontPtr-> O <-> O <-> O <-backPtr
 *  <<<prev     	   next>>>
 * pop here	  		   push here
 */
 
#include <exception>
#include <iostream>
#include "JobQueue.h"

using namespace std;

//Constructor creates an empty list.
//Note invariant: frontPtr==NULL iff backPtr==NULL
JobQueue::JobQueue() {
	frontPtr  = NULL;
	backPtr   = NULL;
	sizeCount = 0;
}

//Deep traverse to delete everything; job pointer heads are not deleted
JobQueue::~JobQueue() {
	if (frontPtr != NULL) {

		Node* current = frontPtr;

		while (current != backPtr) {
			current = current->next;
			delete    current->prev;
		}
	
		delete backPtr;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

//Adds Job j to the backPtr of the list (or frontPtr & backPtr if list is empty).
//Note: does not check if j is already in the JobQueue - will push duplicates!
void JobQueue::push(Job *j) {
	Node* newNode = new Node;
	
	newNode->head = j;
	newNode->next = NULL;
	
	if (empty()) {
		frontPtr      = newNode;
		backPtr       = newNode;
		newNode->prev = NULL;
	} else {
		backPtr->next = newNode;
		newNode->prev = backPtr;
		backPtr       = newNode;
	}
	sizeCount++;
}

//Pop the Job from the frontPtr. Does not return the job, just like an STL JobQueue would.
//Throws error if called while list is empty. Node is freed from heap when popped but the
//job itself is not freed.
void JobQueue::pop() {
	if (frontPtr == NULL) //empty list
		throw runtime_error("Queue: Cannot pop an empty queue");

	if (frontPtr == backPtr) { //check if only one node in list
		delete  frontPtr;
		frontPtr = NULL;
		//list is now completely empty
	} else {
		frontPtr = frontPtr->next;
		delete  frontPtr->prev;
		frontPtr->prev = NULL;
	}
	sizeCount--;
}

//Same as STL
bool JobQueue::empty() {
	return (frontPtr == NULL);
}

//return the front, but does not pop/dequeue the job
Job *JobQueue::front() {
	if (frontPtr == NULL) {
		throw runtime_error("Queue: Cannot peek at the front of an empty queue");
	}

	return frontPtr->head;
}

//Function I added for SharkBatch:
//Force remove a job from somewhere within the JobQueue. Worst case O(n) but
//average case is much better than having to do a full dequeue/enqueue
//cycle from the client's side!
bool JobQueue::force_pop(int pid) {
	Node *n = frontPtr;
	
	if (frontPtr == backPtr && frontPtr != NULL) { //single element list
		delete frontPtr;
		frontPtr = NULL;
		backPtr = NULL;
		return true;
	}
	
	while (n != NULL) {
		if (n->head->get_pid() == pid) {
			n->next->prev = n->prev;
			n->prev->next = n->next;
			
			if (frontPtr == n) {frontPtr = n->next;}
			if (backPtr == n)  {backPtr  = n->prev;}
			
			delete n;
			return true;
		}
		n = n->next;
	}
	return false;
}

//Same as STL
int JobQueue::size() {
	return sizeCount;
}
