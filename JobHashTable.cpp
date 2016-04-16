#include <exception>
#include <stddef.h>
#include <iostream>
#include "JobHashTable.h"

using namespace std;

JobHashTable::JobHashTable() {
	for (int i = 0; i < SIZE; i++) {
		table[i] = NULL;
	}
}

JobHashTable::~JobHashTable() {
	for (int i = 0; i < SIZE; i++) {
		remove(table[i]);
	}
}

void JobHashTable::remove(Node *node) {
	if (node == NULL) {return;}
	
	remove(node->next);
	delete node;
}
		
JobHashTable::Node *JobHashTable::insert(int pid) {
	int key = hash(pid);
	Node *temp = table[key];
	
	table[key] = new Node;
	table[key]->jobPtr = new Job(pid); //this line is the only time that an actual
									   //new job is actually allocated in the heap!
	table[key]->pid = pid;
	table[key]->next = temp;
	
	return table[key];
}

//permanently frees the job itself from memory but keeps the node, that way a complete
//PID can still be looked up to determine whether it exists or not.
void JobHashTable::make_complete(int pid) {
	Node *n = find( pid, table[hash(pid)]);

	delete n->jobPtr;
	n->jobPtr = NULL;
}

//Returns a pointer to the job specified by a pid. Returns NULL if that job has been
//completed. If that job does not yet exist, it will create a new job and insert
//it into the hashtable. The job will be initialized with only a pid and set to
//NOTAJOB when it is created.
Job *JobHashTable::find_or_insert(int pid) {
	return find_or_insert(pid, table[hash(pid)])->jobPtr;
}

Job *JobHashTable::find(int pid) {
	return find(pid, table[hash(pid)])->jobPtr;
}

JobHashTable::Node *JobHashTable::find(int pid, Node *node) {
	if (node == NULL) {	
		throw runtime_error("Searched for a job that does not exist!");
	}
	
	if (node->pid == pid) {
		return node;
	} else {
		find(pid, node->next);
	}
	throw logic_error("Check JobHashTable::find(int pid, Node *node)....?");
}

JobHashTable::Node *JobHashTable::find_or_insert(int pid, Node *node) {
	if (node == NULL) {	
		//create a new Job
		return insert(pid);
	}
	
	if (node->pid == pid) {
		return node;
	} else {
		find(pid, node->next);
	}
	throw logic_error("Check JobHashTable::find(int pid, Node *node)....?");
}

//Given a pointer to a job and a pointer to an IntBST, take all the dependencies in 
//the tree and add the job to the successor list of those dependencies
void JobHashTable::add_successors(Job *job, IntBST *dependencies) {
	vector<int> vec = dependencies->return_vector(); //make a vector out of the Int
	//BST... this is a hack because I don't know how to iterate the IntBST normally
	//but if I have time I'll learn that so I can have O(log n) instead of this
	//which is O(n^2)
	
	for (unsigned i = 0; i < vec.size(); i++) {
		find_or_insert(vec[i])->add_successor(job);
	}
	//by the end of this, all the successors should be updated.
}

//This hash theoretically distributes collisions evenly if the client enters job PIDs
//in order. If there is any worry of collision clustering, simply rewrite this function
// -- no changes to the rest of the code are necessary!
int JobHashTable::hash(int pid) {
	return pid % SIZE;
}

void JobHashTable::premature_kill(int pid) {
	Node *node = table[hash(pid)];
	Node *temp;

	if (node->pid == pid) {
		temp = node->next;
		delete node->jobPtr;
		delete node;
		node->next = temp;
		return;
	}		
	
	node = table[hash(pid)];
	
	while (node->next->pid != pid) {
		node = node->next;
	}
	
	temp = node->next->next;
	delete node->next->jobPtr;
	delete node->next;
	node->next = temp;
}




