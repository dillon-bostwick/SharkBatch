#include <exception>
#include <iostream>
#include "Job.h"

using namespace std;

//A "latent" job is a job with a pid and nothing else
Job::Job(int pid) {
	this->pid = pid;
	this->status = LATENT;
}

Job::~Job() {
	//TODO
}

void Job::prepare(int execTime, int resources) {
	if (this->status != LATENT) {throw runtime_error("Only initialize latent jobs");};
	
	this->execTime = execTime;
	this->originalExecTime = execTime;
	this->resources = resources;
	this->status = WAITING;
}

int Job::decrement_time(int time) {
	if (execTime <= time) {
		status = COMPLETE;
		return execTime;
	} else {
		execTime -= time;
		return time;
	}
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

void Job::print_successors() {
	if (successors.empty()) {
		cout << "N/A" << endl;
	}
	
	for (unsigned i = 0; i < successors.size(); i++) {
		cout << successors[i]->get_pid() << endl;
	}
}

void Job::print_dependencies() {
	if (dependencies.empty()) {
		cout << "N/A" << endl;
	}
	
	for (unsigned i = 0; i < dependencies.size(); i++) {
		cout << dependencies[i]->get_pid() << endl;
	}
}

JobList *Job::get_successors() {
	return &successors;
}

JobList *Job::get_dependencies() {
	return &dependencies;
}

bool Job::no_successors() {
	return successors.empty();
}

void Job::remove_dependency(int pid) {
	for (unsigned i = 0; i < dependencies.size(); i++) {
		if (dependencies[i]->get_pid() == pid) {
			//O(1) solution from Stack Overflow
			swap(dependencies[i], dependencies.back());
			dependencies.pop_back();
		}
	}
}

void Job::add_dependency(Job* j) {
	dependencies.push_back(j);
}

bool Job::no_dependencies() {
	return dependencies.empty();
}

int Job::get_latency() {
	return clockBegin - clockInsert;
}

int Job::get_response() {
	return clockComplete - clockInsert;
}

int Job::get_original_exec() {
	return originalExecTime;
}

int Job::get_turnaround() {
	return clockComplete - clockBegin;
}

void Job::set_clock_insert(int time) {
	clockInsert = time;
}

void Job::set_clock_begin(int time) {
	clockBegin = time;
}

void Job::set_clock_complete(int time) {
	clockComplete = time;
}

void Job::increment_deep_success() {
	deepSuccessorCount++;
}

int Job::get_deep_success() {
	return deepSuccessorCount;
}


