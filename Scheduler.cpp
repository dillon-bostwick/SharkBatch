/*
 * Scheduler.cpp
 * by Dillon Bostwick
 * see Scheduler.h for details
 */

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <curses.h>
#include "Scheduler.h"
#include "CursesHandler.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructing and destructing //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Create the runs by initializing runs as a vector of queues, the size of which is
// specified by the parameter. The quanta are initializing with Q0 starting as BASE
// time, and subsequent priorities have DIFF_QUANTUM less time than the priority beneath
// them. There cannot be more priorities than BASE_QUANTUM / DIFF_QUANTUM.
//
Scheduler::Scheduler(int baseQuantum, int numQueues, bool varyQuanta, bool chainWeighting) {
	if (numQueues > baseQuantum) {
		throw logic_error("baseQuantum time must be larger than numQueues");
	}
	
	//create the runs vector
	runs.resize(numQueues);
	
	this->BASE_QUANTUM    = baseQuantum;
	this->VARY_QUANTA     = varyQuanta;
	this->CHAIN_WEIGHTING = chainWeighting;

	//The win object is already implicitly initialized with a Scheduler. We still need
	//to call wireframe, which creates the UI skeleton
	win.wireframe(numQueues);
	win.mode_bar (varyQuanta, chainWeighting);
	
	//other Scheduler data get initialized
	memoryUsed    = 0;
	runClock      = 0;
	totalComplete = 0;
	
	win.console_bar("Initialization successful");
	win.console_bar(1, "Base quantum: %d",     baseQuantum);
	win.console_bar(2, "Number of queues: %d", numQueues);
}

Scheduler::~Scheduler() {
	//Find every job ever allocated via the jobs hash table, and free that job from memory
	//Note: the JobHashTable has it's own destructor that frees the "buckets" in the array
	//But Scheduler always originally allocates new jobs when they are created
	jobs.destroy_all_jobs();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Run loop //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//Iterate through the Multilevel Feedback Queue and handle user input from the main menu
void Scheduler::run() {
	//curses_startup();

	paused = true;
	exit   = false;
	char inputChar;
		
	//print the initial states of all UI bars to the screen
	win.main_menu();
	win.paused_bar(true);
	
	while (!exit) {
		//if not paused, run a process iteration
		if (!paused) {
			move_from_waiting();
			
			if (!find_next_priority()) {
				win.clear_console();
				win.console_bar("No processes currently running");
			} else {
				process_job();
			}
		}
		
		//check if the user inputted anything
		if ((inputChar = getch()) != ERR) {
			win.paused_bar(true);
			win.blocking_on(); //wait for user input when expected
			main_menu_input(inputChar); //process the request
			win.blocking_off(); //turn off blocking; input will be received asynchronously
			win.main_menu(); //print the main menu again
			win.paused_bar(paused);
		}
	}
	win.~CursesHandler(); //Returns the terminal to normal command line view
}






//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// Functions that handle scheduling and processing ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//sets priority to the highest priority that is not empty, and set current to the next
//jobs to be processed. Return false if there are no processes to run.
bool Scheduler::find_next_priority() {
	priority = runs.size() - 1;
		
	while (priority >= 0) {
		if (!runs[priority].empty()) {		
			current = runs[priority].front();
			break;
		}
		priority--;
	}
	
	return (priority != -1);
}
		
//add from waitingOnMem queue until MAX_MEMORY is reached. This can be a problem
//because it always takes from the front of the queue; it does not scroll through
//with a greedy algorithm looking for all jobs that are less than max. So if one
//job uses all of memory there is a huge problem.
//
//This is sometimes called the "convoy effect" and I am working on figuring
//out a cool solution...	
void Scheduler::move_from_waiting() {
	while (!waitingOnMem.empty() && 
		    waitingOnMem.front()->get_resources() + memoryUsed <= MAX_MEMORY) {
				start_processing(waitingOnMem.front());
				waitingOnMem.pop();
	}
}

//Call when a job is ready to process through the multilevel feedback queues. Set
//status from Job::WAITING to RUNNING, push it to the highest priority queue, and add the 
//resources to memory
void Scheduler::start_processing(Job *new_process) {
	win.feed_bar("Job #%d: Began processing", waitingOnMem.front()->get_pid()); //print
	new_process->set_status(Job::RUNNING);
	runs[runs.size() - 1].push(new_process); //add to the highest level priority
	memoryUsed += new_process->get_resources(); //add resources to memory
	new_process->set_clock_begin(runClock); //record runClock time (for statistics)
}

//Called when current has finished processing in it's allocated time slice. (Execute
//similar tasks to above)
void Scheduler::complete_processing() {
	win.feed_bar("Job #%d: completed", current->get_pid()); //print to feed
	memoryUsed -= current->get_resources(); //take resources off memory
	runs[priority].pop(); //pop from the queue
	totalComplete++; //increment the Job::COMPLETE counter (used for statistics)
	current->set_clock_complete(runClock); //record runClock time (for statistics)
	update_stats(); //update the statistics bar
	update_successors();//remove dependents from all successors & run eligible successors
}

//Given that current and priority are already set, determine the time slice and run
//the clock / sleep / decrement execTime by that slice. If the job finished, call
//complete_processing(), otherwise, move it to the appropriate place in the MLFQ
void Scheduler::process_job() {
	int slice = BASE_QUANTUM;
	//Given the mode, we determine the slice based off the original quantum different.
	//First, if VARY_QUANTA, higher priorities have shorter quanta, and secondly, if
	//CHAIN_WEIGHTING, the slice is factored by the longest chain number of the job
	if (VARY_QUANTA) {
		slice -= (BASE_QUANTUM / runs.size()) * priority;
	}
	
	if (CHAIN_WEIGHTING) {
		slice *= current->get_longest_chain() + 1;
	}
		
	//"run" current (i.e. decrement the job's remaining execTime) for a time slice that
	//is as long as current's priority's time quantum will allow OR until complete
	runClock += current->decrease_time(slice);
	std::this_thread::sleep_for(std::chrono::microseconds(JIFFIE_TIME * slice));
		
	output_status(slice); //update the status bar

	if (runs[priority].front()->get_status() == Job::COMPLETE) { //completed during slice
		complete_processing();
	} else { //job gets bumped down a priority unless already in round robin base
		runs[priority].pop();
		
		if (priority > 0) {
			priority--;
		}
		runs[priority].push(current);
	}
}

//Go through current's successors, remove current's PID from all of the successor's
//dependency lists, then, if dependency list is empty, insert that successor into
//the runs
void Scheduler::update_successors() {
	Job::JobList *successors = current->get_successors();

	for (unsigned i = 0; i < successors->size(); i++) {
		successors->at(i)->remove_dependency(current->get_pid());
		
		if (successors->at(i)->no_dependencies()) {
			waitingOnMem.push(successors->at(i));
		}
	}
}

//Longest chain algorithm. A job's longest chain is the net jobs that must be completed
//to finish the longest possible chain of successors. This method starts with a new
//job j that was added, and recalculates the times of all jobs that are in j's chain of
//dependencies -- note j is NOT the job for which we want to recalculate the longest chain
//variable. Note also that we are computing # of jobs, not net burst time; see ReadMe
//for why this makes more sense
//
//No help from online resources; it really is not very complex.
//I loosely based my mindset on Dijkstra. Not sure if this is the optimal complexity
//or not but I'd rather use recursion than determine whether another solution is more
//optimized without relying on the runtime stack.
void Scheduler::deep_search_update(Job *j, int num) {
	//Recurse through all of j's dependents with an incremented num (base case if no
	//dependents exist)
	for (unsigned i = 0; i < j->get_dependencies()->size(); i++) {
		deep_search_update(j->get_dependencies()->at(i), num + 1);
	}
	
	//If the passed num is greater than j's current longest chain, then set the
	//new longest. (NB: Job's constructor always initiates longest chain to 0)
	if (num > j->get_longest_chain()) {
		j->set_longest_chain(num);
	}
}













//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//Functions that handle IO and main menu requests/////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//Main menu input character interpreter
void Scheduler::main_menu_input(char input) {
	switch (input) {
		case 'e':
			exit = true;
			break;
		case 'p':
			paused = !paused;
			win.paused_bar(paused);
			break;
		case 'a':
			make_job_from_cin();
			break;
		case 'l':
			lookup_from_input();
			break;
		case 'k':
			kill_job();
			break;
		case 'f':
			add_from_file();
			break;
		default:
			win.clear_console();
			win.console_bar("Must input from list of characters above.");
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//Functions that help create new jobs
//////////////////////////////////////////////////////////////////////////////////////////

//Handles a request to add a list of jobs from an external file source.
void Scheduler::add_from_file() {
	ifstream inFile;
	char     fileName[256]; //Just using a C style string to begin with for simplicity
	bool     fail = false;
	
	win.clear_console();

	while (true) {
		win.menu_bar("Enter a file name: ");
		getstr(fileName); //Of NCurses API; dont need CursesHandler because already C str
		inFile.open(fileName);
		
		if (inFile.fail()) {
			win.console_bar("File not found.");
		} else {
			break;
		}
	}
	
	win.console_bar("Loading a large file may take a while if Chain Weighting Mode is"
					" enabled....");
	while (!inFile.eof()) {
		if (!make_job_from_line(inFile)) {
			fail = true;
		}	
	}
	
	inFile.close();
	
	if (fail) {
		win.console_bar("Loaded with some errors (see feed): ", fileName);
	} else {
		win.console_bar("Successfully loaded: ", fileName);
	}
	refresh();
}

//Add a job from cin. Record execTime, resources, and then call read_dependencies()
//to generate a pointer to a IntBST that includes every PID of each dependency
//that the user specifies via cin. Insert the job into the JobHashTable "jobs". Then,
//check if there are any dependencies. If the dependency tree is already completely
//empty, then start processing the job immediately (i.e. it inters into the highest
//priority of the Multilevel Feedback Queue), otherwise, it waits as "WAITING"
//until the dependencies vector is completely empty, in which case it enters waitingOnMem
void Scheduler::make_job_from_cin() {
	int pid	 	  = cin_pid();
	int execTime  = cin_exec_time();
	int resources = cin_resources();
	Job *j		  = jobs.find(pid);
	
	win.clear_console();

	//We now prepare the job with the given information. This will automatically set the 
	//job status from Job::LATENT to Job::WAITING
	j->prepare(execTime, resources);
	
	//Now we read all dependencies and add them
	read_dependencies(j, false, cin);
	
	//If j has no dependencies, we push it immediately to waitingOnMem, where it waits
	//to be pushed into the runs
	if (j->no_dependencies()) {
		waitingOnMem.push(j);
	} //else, we don't do anything. j will sit in "jobs" until its dependencies
	//list is empty, in which case process_job() will take care of pushing to waitingOnMem
	
	j->set_clock_insert(runClock);
	
	win.clear_console();
	win.console_bar(0, "Created new job: #%d", pid);
	win.console_bar(1, "Execution time: %d", j->get_exec_time());
	win.console_bar(2, "Resources required: %d", j->get_resources());
	win.console_bar(3, "Dependents:");
	win.console_bar(4, j->get_dependencies());
}

//Same as make_job_from_cin but everything is processed from an istream instead of user
//input. See above for more details and comments explaining code segments
bool Scheduler::make_job_from_line(istream &inFile) {
	int  pid;
	int  execTime;
	int  resources;
	Job *j;
	
	win.clear_console();
	
	inFile >> pid;
	j = jobs.find(pid);
	
	if (j != NULL && j->get_status() != Job::LATENT)
		return file_error("job already exists", pid, inFile);
	
	if (j == NULL) {
		j = new Job(pid);
		jobs.insert(j);
	}

	inFile >> execTime;
	if (execTime <= 0)
		return file_error("execution time must be positive", pid, inFile);

	inFile >> resources;
	if (resources > MAX_MEMORY) 
		return file_error("cannot use more than MAX MEMORY.", pid, inFile);
	if (resources < 0)
		return file_error("resources cannot be negative.", pid, inFile);

	j->prepare(execTime, resources);
	read_dependencies(j, true, inFile);

	if (j->no_dependencies()) {
		waitingOnMem.push(j);
	}
	
	j->set_clock_insert(runClock);
	win.feed_bar("Created new job PID #%d", pid);
	return true;
}

//Take a list of PIDs from cin. For each PID, check the "jobs" JHT to see
//whether it is already Job::COMPLETE. If it isn't already Job::COMPLETE, add it to a
//JHT called dependencies. Also, append the job pointer j to that PID's successor
//list. When finished, return the dependencies JHT. (NOTE: there are two different
//uses of the JHT class here)
void Scheduler::read_dependencies(Job *j, bool externalFile, istream &inFile) {
	int  pid = -2;
	Job *dependentJob;
	
	if (!externalFile) {
		win.menu_bar("Enter dependencies, enter -1 when finished: ");
	}
	
	for (int i = 1; true; i++) { //runs for true because sentinel to breaks loop
		//get pid either from cin or from istream
		if (externalFile) {
			inFile >> pid;
		} else {
			pid = win.get_int_input();
			win.keep_cursor_in_menu(i);
		}
		
		if (pid == -1) {break;} //just using a simple -1 sentinel
	
		dependentJob = jobs.find(pid);
		//If the job specified does not already exist in jobs, we create a new job
		//that is "Job::LATENT" -- i.e., we need to keep track of it as a job that could 
		//potentially get prepared by the client later. For now, it just has a pid and
		//a list of successors	
		if (dependentJob == NULL) {
				dependentJob = new Job(pid);
				jobs.insert(dependentJob);
		}
		//If the dependentJob is already Job::COMPLETE, then we don't care and we just
		//ignore that input
		if (dependentJob->get_status() != Job::COMPLETE) {
			//Otherwise we insert a pointer to the job into the dependencies table
			j->add_dependency(dependentJob);
			//And we also need to add our new job to the given job's successor list
			dependentJob->add_successor(j);
			//Now we run the deep search function on the new dependent job (see
			//comments for the deep_search_increment function definition for details).
			//Note that we pass the second parameter as j's current longest chain, which
			//might not necessarily be 0 if it was initialized out of a latent state,
			//plus 1 because we are adding another chain level either way
			if (CHAIN_WEIGHTING) {
				deep_search_update(dependentJob, j->get_longest_chain() + 1);
			}
		}
	}
}

bool Scheduler::file_error(string str, int pid, istream &inFile) {
	win.feed_bar("Error reading file: PID #%d: " + str, pid);
	inFile.ignore(256, '\n');
	return false;
}

int Scheduler::cin_pid() {
	int pid;
	Job *j;
	
	do {
		win.menu_bar("To add a job, enter PID: ");
		pid = win.get_int_input();

		j = jobs.find(pid);
		
		if (j != NULL && j->get_status() != Job::LATENT) {
			win.console_bar("PID #%d already exists. Enter a different PID.", pid);
		} else {
			if (j == NULL) {
				//Create a new Job and insert it into the hashtable
				j = new Job(pid);
				jobs.insert(j);
				win.console_bar("Creating new job PID #%d", pid);
			} //else, it already exists as latent
			return pid;
		}
	} while (true);
}

int Scheduler::cin_exec_time() {
	int execTime;
	
	do {
		win.menu_bar("Expected execution (burst) time: ");
		execTime = win.get_int_input();
		if (execTime <= 0) {
			win.console_bar("Execution time must be positive. Enter a different time.");
		} else {
			return execTime;
		}
	} while (true);
}

int Scheduler::cin_resources() {
	int resources;
	
	do {
		win.menu_bar("Resources needed: ");
		resources = win.get_int_input();
		if (resources > MAX_MEMORY) {
			win.console_bar("Cannot use more than MAX MEMORY. Enter a different amount.");
		} else if (resources < 0) {
			win.console_bar("Resources cannot be negative. Enter a different amount.");
		} else {
			return resources;
		}
	} while (true);
}




//////////////////////////////////////////////////////////////////////////////////////////
//Find a job from input
//////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::lookup_from_input() {
	int pid;
	
	win.menu_bar("To find a job, enter PID: ");
	pid = win.get_int_input();

	win.clear_console();
	
	if (jobs.find(pid) == NULL) {
		win.console_bar("Error: this PID does not exist anywhere");
	} else {
		job_on_console(jobs.find(pid));
	}
}

void Scheduler::job_on_console(Job *j) {
	win.console_bar("Job #%d:", j->get_pid());
	
	switch (j->get_status()) {
		case Job::COMPLETE:
			win.console_bar(1, "Job::COMPLETE");
			break;
		case Job::RUNNING:
			win.console_bar(1, "RUNNING");
			win.console_bar(2, "Burst time remaining: %d", j->get_exec_time());
			win.console_bar(3, "Resources allocated: %d", j->get_resources());
			win.console_bar(4, "Successors: ");
			win.console_bar(5, j->get_successors());
			win.console_bar(6, "Longest chain: %d", j->get_longest_chain());
			break;
		case Job::WAITING:
			win.console_bar(1, "Job::WAITING");
			win.console_bar(2, "Dependents:");
			win.console_bar(3, j->get_dependencies());
			win.console_bar(4, "Successors:");
			win.console_bar(5, j->get_successors());
			win.console_bar(6, "Longest chain: %d", j->get_longest_chain());
			break;
		case Job::LATENT:
			win.console_bar(1, "Job::LATENT");
			win.console_bar(2, "Successors:");
			win.console_bar(3, j->get_successors());
			win.console_bar(4, "Longest chain: %d", j->get_longest_chain());
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//Kill a job from input
//////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::kill_job() {
	int pid;
	Job *j;
	
	win.clear_console();
	
	win.menu_bar("To kill a job, enter PID: ");
	pid = win.get_int_input();
	
	j = jobs.find(pid);
	
	if (j == NULL) {
		win.console_bar("Error: this PID does not exist anywhere");
		return;
	}
	
	if (j->get_status() == Job::LATENT) {
		win.console_bar("Error: this job cannot be killed at this time");
	}

	if (j->get_status() == Job::COMPLETE) {
		win.console_bar("Error: this job already Job::COMPLETEd.");
		return;
	} else if (!j->no_successors()) {
		win.console_bar("Warning: some jobs are dependent on the completion of this job to run. ");
		win.menu_bar("Kill anyway? y/n");
		
		if (win.get_y_n()) {
			convert_to_latent(j);
		} else {
			return;
		}
	}else if (j->get_status() == Job::WAITING) {
		win.console_bar("Warning: this job has not yet began processing.");
		win.menu_bar("Remove from schedule anyway? y/n: ");
		
		if (win.get_y_n()) {
			convert_to_latent(j);		
		} else {
			return;
		}
	} else {
		jobs.remove(pid); //remove j from the jobs hashtable
		delete j; //permanently free j from the heap
		//now find j in the runs and remove it so that the runs won't try to dereference
		//the dead pointer
		for (unsigned i = 0; i < runs.size(); i++) {
			if (runs[i].force_pop(pid)) {
				break;
			}
		}
	}
	win.console_bar("Job #%d killed prematurely.", pid);
}

//we need to copy the successors, delete j, create a new Job::LATENT job, then update
//the new Job::LATENT job with the successors
void Scheduler::convert_to_latent(Job *j) {
	Job::JobList temp;
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

//Other output printers///////////////////////////////////////////////////////////////////

void Scheduler::update_stats() {
	//add current's runClock times into totals
	totalLatency += current->get_latency();
	totalTurnaround += current->get_turnaround();
	totalResponse += current->get_response();
	totalTurnPerBurst += totalTurnaround / current->get_original_exec();
	totalLatencyPerBurst += totalLatency / current->get_original_exec();
	
	//print all the statistics to 3 decimal places
	win.stats_bar(0, "Throughput: %g",        	    (double) totalComplete   / runClock);
	win.stats_bar(1, "Average latency: %g",         (double) totalLatency    / totalComplete);
	win.stats_bar(2, "Average response time: %g",   (double) totalResponse   / totalComplete);
	win.stats_bar(3, "Average turnaround time: %g", (double) totalTurnaround / totalComplete);
	win.stats_bar(4, "Average turnaround per burst time: %g",totalTurnPerBurst / totalComplete);
	win.stats_bar(5, "Average latency per burst timet: %g",  totalLatencyPerBurst / totalComplete);
	win.stats_bar(6, "Total jiffies processed: %g", 		 runClock);
}

void Scheduler::output_status(int slice) {
	//for status_bar, the leading integer parameter is a row, not a column, unless two
	//leading integers are specified, in which case it is row, column, str...
	//This is the only CursesHandler function like this!
	
	win.clear_status_bar();
	win.status_bar(0, "Queue size:");
	
	//print the MLFQ queue sizes
	for (unsigned i = 0; i < runs.size(); i++) {
		win.status_bar(15 + i * 4, "%d", runs[i].size()); //print the queue size
		win.status_bar(17 + i * 4, "|");
	}
	
	win.status_bar(15 + (runs.size() * 4), "%d",      waitingOnMem.size());//waiting queue
	win.status_bar(1, 0, "Total memory occupied: %d", memoryUsed);		   //memory used
	
	win.core_bar(0, "PID: %d ", 			    current->get_pid());
	win.core_bar(1, "Priority: %d",			    priority);
	win.core_bar(2, "Burst time remaining: %d", current->get_exec_time());
	win.core_bar(3, "Time slice allocated: %d", slice);
}

