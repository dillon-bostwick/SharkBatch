#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include "Scheduler.h"

using namespace std;

/* Create the MLFQ by initializing runs as a vector of queues, the size of which is
 * specified by the parameter. The quantums are initializing with Q0 starting as BASE
 * time, and subsequent priorities have DIFF_QUANTUM less time than the priority beneath
 * them. There cannot be more priorities than BASE_QUANTUM / DIFF_QUANTUM.
 */
Scheduler::Scheduler(int numQueues) {
	if (numQueues > BASE_QUANTUM / DIFF_QUANTUM) {
		throw logic_error("Note: number of priorities must be less than BASE QUANTUM /"
		"DIFFERENCE BETWEEN QUANTUMS, or else some queues would have non-positive quantums."
		" Check your command line argument.");
	}
	
	//create the runs vector
	runs.resize(numQueues);
	
	//create the quants vector
	for (int i = 0; i < numQueues && i < BASE_QUANTUM / DIFF_QUANTUM; i++) {
		quants.push_back(BASE_QUANTUM - (i * DIFF_QUANTUM));
	}
	
	//other data get initialized
	memoryUsed = 0;
}

Scheduler::~Scheduler() {
	//TODO
}

//Iterate through the Multilevel Feedback Queue until the user says to end the program
void Scheduler::run() {
	
	while (user_input()) {
		//add from waitingOnMem queue until MAX_MEMORY is reached. This can be a problem
		//because it always takes from the front of the queue; it does not scroll through
		//with a greedy algorithm looking for all jobs that are less than max. So if one
		//job uses all of memory there is a huge problem.
		//
		//This is sometimes called the "convoy effect" and I am working on figuring
		//out a cool solution...
		
		while (!waitingOnMem.empty() && 
		 	   waitingOnMem.front()->get_resources() + memoryUsed <= MAX_MEMORY) {
			start_processing(waitingOnMem.front());
			waitingOnMem.pop();
		}

		//set current to the most important job to run and set "priority" to the priority
		//of that job
		
		priority = runs.size() - 1;
		
		while (priority >= 0) {
			if (!runs[priority].empty()) {		
				current = runs[priority].front();
				break;
			}
			priority--;
		}

		if (priority == -1) {
			cout << "No processes to run. Nothing happened." << endl;
		} else {
			process_job();
			print_status();
		}
		
	}
}

//Printer used by processor
void Scheduler::print_status() {
	cout << "Queue:  ";
	for (unsigned i = 0; i < runs.size(); i++) {
		cout << "Q" << i << "   ";
	}
	
	cout << "W" << endl << "Size:   ";
	
	for (unsigned i = 0; i < runs.size(); i++) {
		cout << runs[i].size() << "    ";
	}
	
	cout << waitingOnMem.size() << endl << "just processed job #" << current->get_pid();

	if (current->get_exec_time() <= 0) {
		cout << ", which completed in it's allocated CPU time slice." << endl;
	} else {
		 cout << " which is now in priority Q" << priority << " with "
		 << current->get_exec_time() << " time remaining. Q" << priority
		 << " has a quantum of " << quants[priority] << "." << endl;
	}
		cout << "Memory occupied by all current processes: " << memoryUsed << "KB" << endl;
}

//User input reader used by processor
bool  Scheduler::user_input() {
	
	char input;
	
	while (true) {
		cout << "INPUT: r = run. a = add job. l = lookup. k = kill. e = end: ";
		cin >> input;
	
		switch (input) {
			case 'e': return false;
			case 'r': return true;
			case 'a':
				make_job_from_input();
				break;
			case 'l':
				lookup_from_input();
				break;
			case 'k':
				kill_job();
				break;
			default:
				cout << "Must input from list of characters above." << endl;
				break;
		}
	}
}





//Add a job from cin. Record execTime, resources, and then call read_dependencies()
//to generate a pointer to a IntBST that includes every PID of each dependency
//that the user specifies via cin. Insert the job into the JobHashTable "jobs". Then,
//check if there are any dependencies. If the dependency tree is already completely
//empty, then start processing the job immediately (i.e. it inters into the highest
//priority of the Multilevel Feedback Queue), otherwise, it enters the JobHeap
//"waiting" until the IntBST dependencies is completely empty, in which case it is
//popped from the JobHeap.
void Scheduler::make_job_from_input() {
	int pid;
	int execTime;
	int resources;
	Job *j;
	
	cout << "Add a job." << endl;
	
	//Get the PID from cin
	do {
		cout << "PID: ";
		cin >> pid;
		
		j = jobs.find(pid);
		
		if (j != NULL && j->get_status() != LATENT) {
			cout << "PID already exists. Enter a different PID." << endl;
		} else {
			if (j == NULL) {
				//Create a new Job and insert it into the hashtable
				j = new Job(pid);
				jobs.insert(j);
			} //else, it is already exists as LATENT.
			break;
		}
	} while (true);
	
	//Get the execTime from cin
	do {
		cout << "Execution time: ";
		cin >> execTime;
		if (execTime <= 0) {
			cout << "Execution time must be positive. Enter a different time." << endl;
		} else {
			break;
		}
	} while (true);
	
	//Get the resources from cin
	do {
		cout << "Resources needed: ";
		cin >> resources;
		if (resources > MAX_MEMORY) {
			cout << "cannot use more than MAX MEMORY. Enter a different amount." << endl;
		} else if (resources < 0) {
			cout << "Resources cannot be negative. Enter a different amount." << endl;
		} else {
			break;
		}
	} while (true);

	//We now prepare the job with the given information. This will automatically set the 
	//job status from LATENT to WAITING
	j->prepare(execTime, resources);
	
	//Now we read all dependencies from cin and add them
	read_dependencies(j);
	
	//If j has no dependencies, we push it immediately to waitingOnMem, where it waits
	//to be pushed into the MLFQ
	if (j->no_dependencies()) {
		waitingOnMem.push(j);
	} //else, we don't do anything. j will sit in "jobs" until its dependencies
	//list is empty, in which case process_job() will take care of pushing to waitingOnMem
}

//Take a list of PIDs from cin. For each PID, check the "jobs" JHT to see
//whether it is already complete. If it isn't already complete, add it to a
//JHT called dependencies. Also, append the job pointer j to that PID's successor
//list. When finished, return the dependencies JHT. (NOTE: there are two different
//uses of the JHT class here)
void Scheduler::read_dependencies(Job *j) {
	int pid;
	Job *dependentJob;
	
	cout << "Enter dependencies, enter -1 when finished:" << endl;
	
	while (std::cin) {
		cin >> pid;
		
		if (pid == -1) {break;} //this is a little hack that works for now
								//because using eof() and ctl-D was being buggy
								
		dependentJob = jobs.find(pid);

		//If the job specified does not already exist in jobs, we create a new job
		//that is "latent" -- i.e., we need to keep track of it as a job that could 
		//potentially get prepared by the client later. For now, it just has a pid and
		//a list of successors	
		if (dependentJob == NULL) {
				dependentJob = new Job(pid);
				jobs.insert(dependentJob);
		}
		
		//If the dependentJob is already complete, then we don't care and we just ignore
		//that input
		if (dependentJob->get_status() != COMPLETE) {
			//Now we just insert a pointer to the job into the dependencies table
			j->add_dependency(dependentJob);
		
			//And we also need to add our new job to the given job's successor list
			dependentJob->add_successor(j);
		}
	}
}



//Call when a job is ready to process through the multilevel feedback queues. Set
//status from WAITING to RUNNING, push it to the highest priority queue, and add the 
//resources to memory
void Scheduler::start_processing(Job *new_process) {
		new_process->set_status(RUNNING);
		runs[runs.size() - 1].push(new_process); //add to the highest level priority
		memoryUsed += new_process->get_resources();
}

void Scheduler::lookup_from_input() {
	int pid;
	Job *j;
	
	cout << "To find a job, enter PID: ";
	cin >> pid;
	
	j = jobs.find(pid);
	
	if (j == NULL) {
		cout << "Error: this PID does not exist anywhere" << endl;
		return;
	}
	
	cout << "Status: ";
	
	if (j->get_status() == COMPLETE) {
		cout << "COMPLETE" << endl;
	} else if (j->get_status() == RUNNING) {
		cout << "RUNNING" << endl
		     << "Resources allocated: " << j->get_resources() << " (All processes: "
		     << memoryUsed << ". Max: " << MAX_MEMORY << ")" << endl 
		     << "Successors:" << endl;
		j->print_successors();
	} else if (j->get_status() == WAITING) {
		cout << "WAITING" << endl << "Dependents:" << endl;
		j->print_dependencies();
		cout << "Successors:" << endl;
		j->print_successors();
	} else {
		cout << "LATENT" << endl << "Successors:" << endl;
		j->print_successors(); 
	}
}

//Go through current's successors, remove current's PID from all of the successor's
//dependency trees, then, if dependency tree is empty, insert that successor into
//the MLFQ
void Scheduler::update_successors() {
	JobList *successors = current->get_successors();

	for (unsigned i = 0; i < successors->size(); i++) {
		successors->at(i)->remove_dependency(current->get_pid());
		
		if (successors->at(i)->no_dependencies()) {
			waitingOnMem.push(successors->at(i));
		}
	}
}

void Scheduler::process_job() {
	int slice;
	
	//"run" current (aka decrement the job's remaining execTime) for a time slice that
	//is as long as current's priority's time quantum will allow OR until the current
	//is complete.
	slice = quants[priority]; //set slice to the quantum
	while (current->get_status() != COMPLETE && slice > 0) {
		current->decrement_time();
	
		slice--;	
	}
	
	if (runs[priority].front()->get_status() == COMPLETE) {
		//Job was completed:
		current->set_status(COMPLETE);
		memoryUsed -= current->get_resources(); //take resources off memory
		runs[priority].pop(); //pop from the queue
		update_successors();//remove dependents from all the successors and
		//run eligible successors
		
	} else {
		//if the job wasn't completed then it must have used up it's slice and will now
		//get bumped down a priority UNLESS it is already at priority == 0, in which case
		//it will just get pushed to the back of the baseline queue for infinite round
		//robin until completion (all the long but unimportant processes
		//end up there
			runs[priority].pop();
			
			if (priority > 0) {
				priority--;
			}
				
			runs[priority].push(current);
	}
}

void Scheduler::kill_job() {
	int pid;
	Job *j;
	char cont;
	
	cout << "To kill a job, enter PID: ";
	cin >> pid;
	
	j = jobs.find(pid);
	
	if (j == NULL) {
		cout << "Error: this PID does not exist anywhere" << endl;
		return;
	}
	
	if (j->get_status() == LATENT) {
		cout << "Error: this job cannot be killed at this time" << endl;
	}
	
	if (!j->no_successors()) {
		cout << "Warning: some jobs are dependent on the completion of this job to run. ";
		cout << "Kill anyway? y/n: ";
		cin >> cont;
		if (cont != 'y') {
			return;
		}
	}


	if (j->get_status() == COMPLETE) {
		cout << "Error: this job already completed." << endl;
		return;
	} else if (j->get_status() == WAITING) {
		cout << "Warning: this job has not yet began processing. Remove from schedule "
		        "anyway? y/n: " << endl;
		cin >> cont;
		
		if (cont == 'y') {
			convert_to_latent(j);		
		}
	} else {
		jobs.remove(pid); //remove j from the jobs hashtable
		delete j; //permanently free j from the heap
		//now find j in the MLFQ and remove it so that the MLFQ won't try to dereference
		//the dead pointer
		for (unsigned i = 0; i < runs.size(); i++) {
			if (runs[i].force_pop(pid)) {
				break;
			}
		}
	}
	cout << "Job #" << pid << " killed prematurely." << endl;
}

//we need to copy the successors, delete j, create a new latent job, then update
//the new latent job with the successors
void Scheduler::convert_to_latent(Job *j) {
	JobList temp;
	int pid = j->get_pid();
	
	temp = *(j->get_successors());
	jobs.remove(pid);
	delete j;
	j = new Job(pid);
	jobs.insert(j);
	
	for (unsigned i = 0; i < temp.size(); i++) {
		j->add_successor(temp.at(i));
	}			
}




