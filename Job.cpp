#include <exception>
#include <iostream>
#include "Job.h"

using namespace std;

Job::Job(int pid, int execTime, int resources, IntBST *dependencies) {
	this->pid = pid;
	this->execTime = execTime;
	this->resources = resources;
	this->status = WAITING;
	this->dependencies = dependencies;
}

void Job::decrement_time() {
	execTime--;
	
	if (execTime <= 0) {
		status = COMPLETE;
	}
}

bool Job::is_complete() {
	return status == COMPLETE;
}

int Job::get_pid() {
	return pid;
}

int Job::get_exec_time() {
	return execTime;
}

int Job::get_resources() {
	return resources;
}

void Job::set_status(Status status) {
	this->status = status;
}

Status Job::get_status() {
	return status;
}

void Job::add_successor(Job *j) {
	successors.push_back(j); //always appends to the back of the vector
}

void Job::remove_dependency(int pid) {

	if (!dependencies->remove(pid)) {
		throw runtime_error("Job::remove_dependency(int pid): removal of a"
		"PID from the dependency tree failed, probably because the PID does not"
		"exist in the tree");
	} //else the removal succeeded
}

void Job::print_successors() {
	if (successors.empty()) {
		cout << "N/A" << endl;
	}
	
	for (unsigned i = 0; i < successors.size(); i++) {
		cout << successors[i]->get_pid() << endl;
	}
}

void Job::print_dependencies() {
	dependencies->print();
}

Job *Job::get_successor(int index) {
	return successors[index];
}

int Job::get_successor_size() {
	return successors.size();
}

IntBST *Job::get_dependencies() {
	return dependencies;
}

