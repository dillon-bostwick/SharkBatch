#include <exception>
#include <stddef.h>
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
		
void JobHashTable::insert(Job *job) {
	int key = hash(job->get_pid());
	Node *temp = table[key];
	
	table[key] = new Node;
	table[key]->jobPtr = job;
	table[key]->pid = job->get_pid();
	table[key]->next = temp;
}

//permanently frees the job itself from memory but keeps the node, that way a complete
//PID can still be looked up to determine whether it exists or not.
void JobHashTable::make_complete(int pid) {
	Node *n = find( pid, table[hash(pid)]);

	delete n->jobPtr;
	n->jobPtr = NULL;
}

//Returns a pointer to the job specified by a pid. Returns NULL if that job has been
//completed, and will crash if that pid is nowhere in the table.
Job *JobHashTable::find_job(int pid) {
	return find(pid, table[hash(pid)])->jobPtr;
}

JobHashTable::Node *JobHashTable::find(int pid, Node *node) {
	if (node == NULL) {
		throw runtime_error("Tried to search for a job that does not exist");
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
		find_job(vec[i])->add_successor(job);
	}
	//by the end of this, all the successors should be updated.
}

//REWRITE THIS TO A MORE COMPLEX HASH
int JobHashTable::hash(int pid) {
	return pid % SIZE;
}