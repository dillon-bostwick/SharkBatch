#include <exception>
#include <stddef.h>
#include <iostream>
#include <vector>
#include "JobHashTable.h"

using namespace std;

//create the buckets using DEFAULT_CAP
JobHashTable::JobHashTable() {
	capacity = DEFAULT_CAP;
	buckets = new vector<Job*>[capacity];
	size = 0;
}

//create the buckets with a specific expected capacity
JobHashTable::JobHashTable(int capacity) {
	this->capacity = capacity;
	buckets = new vector<Job*>[capacity];
	size = 0;
}

JobHashTable::~JobHashTable() {
	delete [] buckets;
}

///////////////////////////////////////////////////////////////////////////////////////


//Return a pointer to a job; if the job doesn't exist, returns NULL -- best O(1)
Job *JobHashTable::find(int pid) {
	int key = hash(pid);
	
	for (unsigned i = 0; i < buckets[key].size(); i++) {
		if (buckets[key][i]->get_pid() == pid) {
			return buckets[key][i];
		}
	}
	return NULL;
}

//insert a job pointer -- always O(1)
void JobHashTable::insert(Job *j) {
	double LOAD_FACTOR_THRESHOLD = .8;
	
	buckets[hash(j->get_pid())].push_back(j);
	size++;

	if (((double) size / capacity) >= LOAD_FACTOR_THRESHOLD) {
		expand();
	}
}
		
//Pop a job from the hash buckets (the job still exists in the heap) -- best O(1)
bool JobHashTable::remove(int pid) {
	int key = hash(pid);
	
	for (unsigned i = 0; i < buckets[key].size(); i++) {
		if (buckets[key][i]->get_pid() == pid) {
			//An O(1) removal solution when order doesn't matter -- from Stack Overflow
			swap(buckets[key][i], buckets[key].back());
			buckets[key].pop_back();
			return true;
		}
	}
	return false;
}

//Print to cout, does not sort PIDs in order -- always O(n)
void JobHashTable::print() {
	for (int i = 0; i < capacity; i++) {
		cerr << "Bucket #" << i << ": ";
		for (unsigned j = 0; j < buckets[i].size(); j++) {
			cerr << buckets[i][j]->get_pid() << ",";
		}
	}
}

//Always O(capacity)
bool JobHashTable::is_empty() {
	for (int i = 0; i < capacity; i++) {
		if (!buckets[i].empty()) {
			return false;
		}
	}
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////

int JobHashTable::hash(int pid) {
	return pid % capacity;
}

void JobHashTable::expand() {
	vector<Job*> *newTable;
	
	capacity *= 2;
	
	newTable = new vector<Job*>[capacity];
	
	//rehash everything
	for (int i = 0; i < capacity / 2; i++) {
		for (unsigned j = 0; j < buckets[i].size(); j++) {
			newTable[hash(buckets[i][j]->get_pid())].push_back(buckets[i][j]);
		}
	}
	
	delete [] buckets;
	buckets = newTable;
}





