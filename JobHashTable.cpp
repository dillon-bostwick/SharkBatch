#include <exception>
#include <stddef.h>
#include <iostream>
#include "JobHashTable.h"

using namespace std;

//create the table using DEFAULT_CAP
JobHashTable::JobHashTable() {
	capacity = DEFAULT_CAP;
	table = new vector<Job*>[capacity];
}

//create the table with a specific expected capacity
JobHashTable::JobHashTable(int capacity) {
	this->capacity = capacity;
	table = new vector<Job*>[capacity];
}

JobHashTable::~JobHashTable() {
	delete [] table;
}

///////////////////////////////////////////////////////////////////////////////////////


//Return a pointer to a job; if the job doesn't exist, returns NULL -- best O(1)
Job *JobHashTable::find(int pid) {
	int key = hash(pid);
	
	for (unsigned i = 0; i < table[key].size(); i++) {
		if (table[key][i]->get_pid() == pid) {
			return table[key][i];
		}
	}
	return NULL;
}

//insert a job pointer -- always O(1)
void JobHashTable::insert(Job *j) {
	table[hash(j->get_pid())].push_back(j);
}
		
//Pop a job from the hash table (the job still exists in the heap) -- best O(1)
bool JobHashTable::remove(int pid) {
	int key = hash(pid);
	
	for (unsigned i = 0; i < table[key].size(); i++) {
		if (table[key][i]->get_pid() == pid) {
			//An O(1) removal solution when order doesn't matter -- from Stack Overflow
			swap(table[key][i], table[key].back());
			table[key].pop_back();
			return true;
		}
	}
	return false;
}

//Print to cout, does not sort PIDs in order -- always O(n)
void JobHashTable::print() {
	for (int i = 0; i < capacity; i++) {
		for (unsigned j = 0; j < table[i].size(); j++) {
			cout << table[i][j]->get_pid() << endl;
		}
	}
}

//Always O(capacity)
bool JobHashTable::is_empty() {
	for (int i = 0; i < capacity; i++) {
		if (!table[i].empty()) {
			return false;
		}
	}
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////

int JobHashTable::hash(int pid) {
	return pid % capacity;
}