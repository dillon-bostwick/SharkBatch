/*
 * Job.cpp
 * by Dillon Bostwick
 * see Job.h for details
 */

#include <exception>
#include <iostream>
#include "Job.h"

using namespace std;

//Initialize a job as latent
Job::Job(int pid) {
	this->pid = pid;
	this->status = LATENT;
	
	longestSuccesschain = 0;
}

Job::~Job() {
	//TODO
}

//Operational methods/////////////////////////////////////////////////////////////////////

//Convert a LATENT job to a WAITING job that has an execTime and resources
void Job::prepare(int execTime, int resources) {
	if (this->status != LATENT) {throw runtime_error("Only initialize latent jobs");};
	
	this->execTime = execTime;
	this->originalExecTime = execTime;
	this->resources = resources;
	this->status = WAITING;
}

//Decrease the execTime by the passed time parameter. Set status to COMPLETE if the
//execTime depleted in its allocated time
int Job::decrease_time(int time) {
	if (execTime <= time) {
		status = COMPLETE;
		return execTime;
	} else {
		execTime -= time;
		return time;
	}
}

//Remove a job by PID from the dependency vector
void Job::remove_dependency(int pid) {
	for (unsigned i = 0; i < dependencies.size(); i++) {
		if (dependencies[i]->get_pid() == pid) {
			//O(1) popping solution from Stack Overflow
			swap(dependencies[i], dependencies.back());
			dependencies.pop_back();
		}
	}
}

//Setters and getters/////////////////////////////////////////////////////////////////////

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

Job::Status Job::get_status() {
	return status;
}

void Job::add_successor(Job *j) {
	successors.push_back(j); //always appends to the back of the vector
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

void Job::add_dependency(Job* j) {
	dependencies.push_back(j);
}

bool Job::no_dependencies() {
	return dependencies.empty();
}

int Job::get_original_exec() {
	return originalExecTime;
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

void Job::set_longest_chain(int num) {
	longestSuccesschain = num;
}

int Job::get_longest_chain() {
	return longestSuccesschain;
}

//The follow 3 functions are based off basic scheduler criteria; for a brief overview, see
//http://www.cs.tufts.edu/comp/111/notes/Scheduling.pdf
//For more detailed information, see the ReadMe

int Job::get_turnaround() {
	return clockComplete - clockBegin;
}

int Job::get_latency() {
	return clockBegin - clockInsert;
}

int Job::get_response() {
	return clockComplete - clockInsert;
}


//Methods for testing/////////////////////////////////////////////////////////////////////

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


