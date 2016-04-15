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
void Scheduler::process() {
	int slice;
	bool keepGoing = true;
	
	make_job_from_input();
	
	while (keepGoing) { 
		priority = runs.size();

		//set current to the most important job to run and set "priority" to the priority
		//of that job
		while (priority >= 0) {
			if (!runs[priority].empty()) {
				current = runs[priority].front();
				break;
			}
			priority--;
		}
	
		if (priority == -1) {
			throw runtime_error("Crashing because no current processes waiting in MLFQ.");
		}
			
		//"run" current (aka decrement the job's remaining execTime) for a time slice that
		//is as long as current's priority's time quantum will allow OR until the current
		//is complete.
		slice = quants[priority]; //set slice to the quantum
		while (!current->is_complete() && slice > 0) {
			current->decrement_time();
			
			slice--;	
		}

		//check if the job is completed
		if (runs[priority].front()->is_complete()) {
			//remove the job entirely:
			memoryUsed -= current->get_resources(); //take resources off memory
			runs[priority].pop(); //pop from the queue
			update_successors();//remove dependents from all the successors and
			//run eligible successors
			jobs.make_complete(current->get_pid()); //this frees the job from memory, but the
											  //pid stays in the hashtable forever
		} else if (slice <= 0) {
			//if the job wasn't completed then it must have used up it's slice and will now
			//get bumped down a priority UNLESS it is already at priority == 0, in which case
			//it will just get pushed to the back of the baseline queue for infinite round
			//robin until completion (remember -- all the long but unimportant processes
			//end up there
				runs[priority].pop();
				
				if (priority > 0) {
					priority--;
				}
				
				runs[priority].push(current);
		} else {
			throw logic_error("\nDebug: check the process algorithm...?\n");
		}
		
		
		
		//now we need to print the status of the entire scheduler architecture before we
		//iterate. Right now, I'm giving the user an opportunity for input at the end
		//of every iteration, but hopefully it will eventually be an on-the-fly interruption
		//of the loop. The user can also only add runs between larger slices of processes.
		//They can't add a high priority job on-the-fly while a low priority job is in the
		//middle of a slice and suddenly send the low priority job back to it's queue,
		//because there is a nested loop above that lets a job finish it's entire slice
		//or complete itself before running the I/O. 
		print_status();
		keepGoing = user_input();
	}
}

//Printer used by processor
void Scheduler::print_status() {
	cout << "Queue:  ";
	for (unsigned i = 0; i < runs.size(); i++) {
		cout << "Q" << i << "   ";
	}
	
	cout << endl << "Size:   ";
	
	for (unsigned i = 0; i < runs.size(); i++) {
		cout << runs[i].size() << "    ";
	}
	
	cout << endl << "just processed job #" << current->get_pid();
	
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
		cout << "INPUT: r = run. a = add job. l = lookup. e = end: ";
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
			default:
				throw invalid_argument("Must input from list of characters above");
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
	IntBST *dependencies;
	
	cout << "Add a job." << endl << "PID: ";
	cin >> pid;
	
	cout << "Execution time: ";
	cin >> execTime;
	cout << "Resources (KB): ";
	cin >> resources; 
	cout << "List dependencies, type -1 when finished:" << endl;
	dependencies = read_dependencies();
	
	Job *j = new Job(pid, execTime, resources, dependencies);
	jobs.insert(j); //put the job into the JobHashTable "jobs"
	
	if (dependencies->is_empty()) {
		//no dependencies -- insert into the MLFQ algorithm for immediate processing
		start_processing(j);
	} else {
		//otherwise it needs to enter the waiting heap until it is ready.
		//We just need to append successors to all the jobs on which this job is
		//dependent, then it will sit in the hashtable until it is ready
		jobs.add_successors(j, dependencies);
		
	}
}

//Take a list of PIDs from cin. For each PID, check the JobHashTable "jobs" to see
//whether it is already complete. If it isn't already complete, add it to a IntBST
//called dependencies. Return a pointer to this IntBST.
IntBST *Scheduler::read_dependencies() {
	IntBST *dependencies = new IntBST;
	int pid;
	
	while (true) {
		cin >> pid;
		
		if (pid == -1) {break;} //this is a little hack that works for now
								//because using eof() and ctl-D was being buggy
		
		if (jobs.find_job(pid) != NULL) { //If the dependency the client enters is already
		//completed, then we just exclude it from the dependency vector. We are left
		//with only the dependencies that really matter. (In other words, we search
		//the hashtable using find_job(pid), which returns a pointer to a job if
		//that pid is either waiting or processing, but returns NULL of that job
		//is completed.
			dependencies->insert(pid); //So we need to insert the pid into the
									   //tree
		}
	}
	return dependencies;
}

//Call when a job is ready to process through the multilevel feedback queues. Set
//status to RUNNING, push it to the highest priority queue, then add the resources
//to memory
void Scheduler::start_processing(Job *new_process) {
		new_process->set_status(RUNNING);
		runs[runs.size() - 1].push(new_process); //add to the highest level priority
		memoryUsed += new_process->get_resources(); //add resources to memory
}

void Scheduler::lookup_from_input() {
	int pid;
	Job *j;
	
	cout << "To find a job, enter PID: ";
	cin >> pid;
	
	j = jobs.find_job(pid);
	
	cout << "Status: ";
	
	if (j == NULL) {
		cout << "COMPLETE" << endl;
	} else if (j->get_status() == RUNNING) {
		cout << "RUNNING" << endl;
		cout << "Resources allocated: " << j->get_resources() << endl;
		cout << "Successors:" << endl;
		j->print_successors();
	} else {
		cout << "WAITING" << endl;
		cout << "Dependents:" << endl;
		j->print_dependencies();
		cout << "Successors:" << endl;
		j->print_successors();
	}	
}

//Go through current's successors, remove current's PID from all of the successor's
//dependency trees, then, if dependency tree is empty, insert that successor into
//the MLFQ
void Scheduler::update_successors() {
	for (int i = 0; i < current->get_successor_size(); i++) {
		current->get_successor(i)->get_dependencies()->remove(current->get_pid());
		
		if (current->get_successor(i)->get_dependencies()->is_empty()) {
			start_processing(current->get_successor(i));
		}
	}
}

