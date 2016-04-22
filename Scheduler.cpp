#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <curses.h>
#include "Scheduler.h"
#include "CursesHandler.cpp"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
//Constructing and destructing
//////////////////////////////////////////////////////////////////////////////////////////

/* Create the runs by initializing runs as a vector of queues, the size of which is
 * specified by the parameter. The quanta are initializing with Q0 starting as BASE
 * time, and subsequent priorities have DIFF_QUANTUM less time than the priority beneath
 * them. There cannot be more priorities than BASE_QUANTUM / DIFF_QUANTUM.
 */
 

Scheduler::Scheduler(int numQueues) {
	if (numQueues > BASE_QUANTUM / DIFF_QUANTUM) {
		throw logic_error("Note: number of priorities must be less than BASE QUANTUM /"
		"DIFFERENCE BETWEEN QUANTA, or else some queues would have non-positive quantas."
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
	runClock = 0;
	totalComplete = 0;
	
	curses_startup(numQueues); //initiate a bunch of stuff in order to use NCurses UI environment
}

Scheduler::~Scheduler() {
	printw("\n"); //puts the command line cursor beneath the UI if the program quits
	
	curs_set(1); //make cursor visible again
	endwin(); //return window to normal command line state
	
	//todo --free stuff from memory*/
}

//////////////////////////////////////////////////////////////////////////////////////////
//Main loop of scheduler
//////////////////////////////////////////////////////////////////////////////////////////

//Iterate through the Multilevel Feedback Queue and handle user input from the main menu
void Scheduler::run() {
	//curses_startup();

	paused = true;
	exit = false;
	char inputChar;
	
	//print the initial states of all UI bars to the screen
	main_menu();
	paused_bar(true);
	
	while (!exit) {
		//if not paused, run a process iteration
		if (!paused) {
			move_from_waiting();
			
			if (!find_next_priority()) {
				clear_console();
				console_bar("No processes currently running");
				status_bar();
			} else {
				process_job();
				
			}
		}
		
		//check if the user inputted anything
		if ((inputChar = getch()) != ERR) {
			paused_bar(true);
			blocking_on(); //wait for user input when expected
			main_menu_input(inputChar); //process the request
			blocking_off(); //turn off blocking; input will be received asynchronously
			main_menu(); //print the main menu again
			paused_bar(paused);
		}
	}
}






//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//Functions that handle scheduling and processing runtime
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
//status from WAITING to RUNNING, push it to the highest priority queue, and add the 
//resources to memory
void Scheduler::start_processing(Job *new_process) {
	feed_bar("Job #%d: Began processing", waitingOnMem.front()->get_pid()); //print to feed
	new_process->set_status(RUNNING);
	runs[runs.size() - 1].push(new_process); //add to the highest level priority
	memoryUsed += new_process->get_resources(); //add resources to memory
	new_process->set_clock_begin(runClock); //record runClock time (for statistics)
}

void Scheduler::complete_processing() {
	feed_bar("Job #%d: Completed", current->get_pid()); //print to feed
	memoryUsed -= current->get_resources(); //take resources off memory
	runs[priority].pop(); //pop from the queue
	totalComplete++; //increment the complete counter (used for statistics)
	current->set_clock_complete(runClock); //record runClock time (for statistics)
	update_successors();//remove dependents from all the successors and
	//run eligible successors
}

void Scheduler::process_job() {
	int slice;
	
	//"run" current (aka decrement the job's remaining execTime) for a time slice that
	//is as long as current's priority's time quantum will allow OR until the current
	//is complete.
	slice = quants[priority]; //set slice to the quantum
	
	//"Run" the job
	runClock += current->decrement_time(slice);
	std::this_thread::sleep_for(std::chrono::microseconds(JIFFIE_TIME * slice));
		
	status_bar();

	//Check if the job completed in the allocated time slice	
	if (runs[priority].front()->get_status() == COMPLETE) {
		complete_processing();
		update_stats();
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

//Go through current's successors, remove current's PID from all of the successor's
//dependency lists, then, if dependency list is empty, insert that successor into
//the runs
void Scheduler::update_successors() {
	JobList *successors = current->get_successors();

	for (unsigned i = 0; i < successors->size(); i++) {
		successors->at(i)->remove_dependency(current->get_pid());
		
		if (successors->at(i)->no_dependencies()) {
			waitingOnMem.push(successors->at(i));
		}
	}
}










//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//Functions that handle main menu requests
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//Main menu input interpretor
void Scheduler::main_menu_input(char input) {
	switch (input) {
		case 'e':
			exit = true;
			break;
		case 'p':
			paused = !paused;
			paused_bar(paused);
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
			console_bar("Must input from list of characters above.");
			break;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//Functions that create jobs from ncurses input or an fstream file
//////////////////////////////////////////////////////////////////////////////////////////


//Add a job from cin. Record execTime, resources, and then call read_dependencies()
//to generate a pointer to a IntBST that includes every PID of each dependency
//that the user specifies via cin. Insert the job into the JobHashTable "jobs". Then,
//check if there are any dependencies. If the dependency tree is already completely
//empty, then start processing the job immediately (i.e. it inters into the highest
//priority of the Multilevel Feedback Queue), otherwise, it enters the JobHeap
//"waiting" until the IntBST dependencies is completely empty, in which case it is
//popped from the JobHeap.
void Scheduler::make_job_from_cin() {
	int pid;
	int execTime;
	int resources;
	Job *j;
	
	//clear_console();

	//Get the PID from cin
	do {
		menu_bar("To add a job, enter PID: ");
		pid = getIntInput();

		j = jobs.find(pid);
		
		if (j != NULL && j->get_status() != LATENT) {
			console_bar("PID #%d already exists. Enter a different PID.", pid);
		} else {
			if (j == NULL) {
				//Create a new Job and insert it into the hashtable
				j = new Job(pid);
				jobs.insert(j);
				console_bar("Creating new job PID #%d", pid);
			} //else, it already exists as LATENT.
			break;
		}
	} while (true);
	
	//Get the execTime from cin
	do {
		menu_bar("Expected execution (burst) time: ");
		execTime = getIntInput();
		if (execTime <= 0) {
			console_bar("Execution time must be positive. Enter a different time.");
		} else {
			break;
		}
	} while (true);
	
	//Get the resources from cin
	do {
		menu_bar("Resources needed: ");
		resources = getIntInput();
		if (resources > MAX_MEMORY) {
			console_bar("Cannot use more than MAX MEMORY. Enter a different amount.");
		} else if (resources < 0) {
			console_bar("Resources cannot be negative. Enter a different amount.");
		} else {
			break;
		}
	} while (true);

	//We now prepare the job with the given information. This will automatically set the 
	//job status from LATENT to WAITING
	j->prepare(execTime, resources);
	
	//Now we read all dependencies and add them
	menu_bar("Enter dependencies, enter -1 when finished: ");
	read_dependencies(j, false, cin);
	
	//If j has no dependencies, we push it immediately to waitingOnMem, where it waits
	//to be pushed into the runs
	if (j->no_dependencies()) {
		waitingOnMem.push(j);
	} //else, we don't do anything. j will sit in "jobs" until its dependencies
	//list is empty, in which case process_job() will take care of pushing to waitingOnMem
	
	j->set_clock_begin(runClock);
	
	clear_console();
	console_bar(0, "Created new job: #%d", pid);
	console_bar(1, "Execution time: %d", j->get_exec_time());
	console_bar(2, "Resources required: %d", j->get_resources());
	console_bar(3, "Dependents:");
	console_bar(4, j->get_dependencies());
}

bool Scheduler::make_job_from_line(std::istream &inFile) {
	int pid;
	int execTime;
	int resources;
	Job *j;
	
	//clear_console();

	inFile >> pid;
		
	j = jobs.find(pid);
		
	if (j != NULL && j->get_status() != LATENT) {
		feed_bar("Error reading file: PID #%d: job already exists", pid);
		inFile.ignore(256, '\n'); //From StackOF - tells istream to ignore rest of the line
		return false;
	} else {
		if (j == NULL) {
			//Create a new Job and insert it into the hashtable
			j = new Job(pid);
			jobs.insert(j);
		} //else, it is already exists as LATENT.
	}
	
	inFile >> execTime;
	
	if (execTime <= 0) {
		feed_bar("Error reading file: PID #%d: execution time must be positive.", pid);
		inFile.ignore(256, '\n');
		return false;
	}
	
	inFile >> resources;
	
	if (resources > MAX_MEMORY) {
		feed_bar("Error reading file: PID #%d: cannot use more than MAX MEMORY.", pid);
		inFile.ignore(256, '\n');
		return false;
	} else if (resources < 0) {
		feed_bar("Error reading file: PID #%d: resources cannot be negative.", pid);
		inFile.ignore(256, '\n');
		return false;
	}
	
	//We now prepare the job with the given information. This will automatically set the 
	//job status from LATENT to WAITING
	j->prepare(execTime, resources);
	
	//Now we read all dependencies and add them
	read_dependencies(j, true, inFile);
	
	//If j has no dependencies, we push it immediately to waitingOnMem, where it waits
	//to be pushed into the runs
	if (j->no_dependencies()) {
		waitingOnMem.push(j);
	} //else, we don't do anything. j will sit in "jobs" until its dependencies
	//list is empty, in which case process_job() will take care of pushing to waitingOnMem
	
	//Record the runClock time at which the job was inserted
	j->set_clock_insert(runClock);
	
	return true;
}

//Take a list of PIDs from cin. For each PID, check the "jobs" JHT to see
//whether it is already complete. If it isn't already complete, add it to a
//JHT called dependencies. Also, append the job pointer j to that PID's successor
//list. When finished, return the dependencies JHT. (NOTE: there are two different
//uses of the JHT class here)
void Scheduler::read_dependencies(Job *j, bool externalFile, std::istream &inFile) {
	int pid;
	Job *dependentJob;
	
	while (inFile) {
		if (externalFile) {
			inFile >> pid;
		} else {
			pid = getIntInput();
		}
		
		if (pid == -1) {break;} //sentinel method works for now but not ideal
								
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

//Handles a request to add a list of jobs from an external file source.
void Scheduler::add_from_file() {
	ifstream inFile;
	char fileName[100];
	bool fail = false;

	while (true) {
		menu_bar("Enter a file name: ");
		
		getstr(fileName);

		inFile.open(fileName); // workaround a char array from StackOverflow 
		
		if (inFile.fail()) {
			console_bar("File not found.");
		} else {
			break;
		}
	}
	
	while (!inFile.eof()) {
		if (!make_job_from_line(inFile)) {
			fail = true;
		}	
	}
	
	inFile.close();
	
	if (fail) {
		console_bar("Loaded with some errors (see feed): ", fileName);
	} else {
		console_bar("Successfully loaded: ", fileName);
	}
	
	refresh();
}




//////////////////////////////////////////////////////////////////////////////////////////
//Find a job from input
//////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::lookup_from_input() {
	int pid;
	Job *j;
	
	menu_bar("To find a job, enter PID: ");
	pid = getIntInput();
	
	j = jobs.find(pid);
	
	clear_console();
	
	if (j == NULL) {
		console_bar("Error: this PID does not exist anywhere");
		return;
	}
	
	console_bar("Job #%d:", pid);
	
	switch (j->get_status()) {
		case COMPLETE:
			console_bar(1, "COMPLETE");
			break;
		case RUNNING:
			console_bar(1, "RUNNING");
			console_bar(2, "Burst time remaining: %d", j->get_exec_time());
			console_bar(3, "Resources allocated: %d", j->get_resources());
			console_bar(4, "Successors: ");
			break;
		case WAITING:
			console_bar(1, "WAITING");
			console_bar(2, "Dependents:");
			console_bar(3, j->get_dependencies());
			console_bar(4, "Successors:");
			console_bar(5, j->get_successors());
			break;
		case LATENT:
			console_bar(1, "LATENT");
			console_bar(2, "Successors:");
			console_bar(3, j->get_successors());
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//Kill a job from input
//////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::kill_job() {
	int pid;
	Job *j;
	
	clear_console();
	
	menu_bar("To kill a job, enter PID: ");
	pid = getIntInput();
	
	j = jobs.find(pid);
	
	if (j == NULL) {
		console_bar("Error: this PID does not exist anywhere");
		return;
	}
	
	if (j->get_status() == LATENT) {
		console_bar("Error: this job cannot be killed at this time");
	}

	if (j->get_status() == COMPLETE) {
		console_bar("Error: this job already completed.");
		return;
	} else if (!j->no_successors()) {
		console_bar("Warning: some jobs are dependent on the completion of this job to run. ");
		menu_bar("Kill anyway? y/n");
		
		if (get_y_n()) {
			convert_to_latent(j);
		} else {
			return;
		}
	}else if (j->get_status() == WAITING) {
		console_bar("Warning: this job has not yet began processing.");
		menu_bar("Remove from schedule anyway? y/n: ");
		
		if (get_y_n()) {
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
	console_bar("Job #%d killed prematurely.", pid);
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

//////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::update_stats() {
	//add current's runClock times into totals
	totalLatency += current->get_latency();
	totalTurnaround += current->get_turnaround();
	totalResponse += current->get_response();
	totalTurnPerBurst += totalTurnaround / current->get_original_exec();
	totalLatencyPerBurst += totalLatency / current->get_original_exec();
	
	//print all the statistics to 3 decimal places
	stats_bar(0, "Throughput: %g", (double) totalComplete / runClock);
	stats_bar(1, "Average latency: %g", (double) totalLatency / totalComplete);
	stats_bar(2, "Average response time: %g", (double) totalResponse / totalComplete);
	stats_bar(3, "Average turnaround time: %g", (double) totalTurnaround / totalComplete);
	stats_bar(4, "Average turnaround per burst time: %g", totalTurnPerBurst / totalComplete);
	stats_bar(5, "Average latency per burst timet: %g", totalLatencyPerBurst / totalComplete);
	stats_bar(6, "Total jiffies processed: %g", runClock);
}

   





///////////////////Curses Handler stuff//////////////////////

void Scheduler::status_bar() {

	int STATUS_ROW = 19;
	
	move(STATUS_ROW, 0);
	clrtoeol();

	mvprintw(STATUS_ROW, 0, "Queue size:");
	
	for (unsigned i = 0; i < runs.size(); i++) {
		mvprintw(STATUS_ROW, 15 + i * 4, "%d", runs[i].size()); //print the queue size
		mvprintw(STATUS_ROW, 17 + i * 4, "|");
	}
	
	mvprintw(STATUS_ROW, 20 + (runs.size() * 3), "%d", waitingOnMem.size()); //print waitingonMem size
	
	mvprintw(STATUS_ROW + 2, 0, "Total memory occupied: %d", memoryUsed);
	
	
	core_bar(0, "PID: %d ", current->get_pid());
	core_bar(1, "Priority: %d", priority);
	core_bar(2, "Burst time remaining: %d", current->get_exec_time());
	refresh();
}

