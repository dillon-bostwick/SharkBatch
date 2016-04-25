#ifndef __Scheduler_h__
#define __Scheduler_h__

#include <vector>
#include <fstream>
#include <curses.h>
#include "Job.h"
#include "JobHashTable.h"
#include "JobQueue.h"
#include "CursesHandler.h"

class Scheduler {
	public:
		 Scheduler(int baseQuantum, int numQueues, bool varyQuanta, bool chainWeighting);
		~Scheduler();

    	void run();

	private:
		//constants
    	static const int MAX_MEMORY = 1000;   // KB
    	static const unsigned long JIFFIE_TIME = 100;
    	
    	//Set by constructor and 
	    int BASE_QUANTUM;
	    bool VARY_QUANTA;
		bool CHAIN_WEIGHTING;


    	

    	//Jiffie time is an arbitrary unit of time, which is the minimum unit for which
    	//the CPU must process work. All execTimes of jobs are represented as jiffie units.
    	//This should be entered as microseconds of wallclock time to sleep system
    	//per jiffie

		CursesHandler win; //The window which handles ALL of the scheduler's I/O
    	
    	//Storage
    	std::vector<JobQueue> runs; //A vector of queues of pointers to Jobs

    	JobHashTable jobs; //A hashtable of pointers to all Jobs including those
    					   //that are waiting, running, and completed
    					   
    	JobQueue waitingOnMem; //No dependencies left but not enough memory available
		int memoryUsed; //Total memory used by all current processes
		
		

    	//These are changed for each processor iteration but are stored in private so they
    	//can be accessed everywhere
    	Job *current; //current job being processed
    	int  priority; //current priority of current job
    	bool paused;
    	bool exit;
    	
    	
    	
    	//Used for computing statistics
    	int    runClock;
    	int    totalComplete;
		int    totalLatency;
		int    totalTurnaround;
		int    totalResponse;
		double totalTurnPerBurst;
		double totalLatencyPerBurst;

    	//private functions and helpers
    	void move_from_waiting();
    	bool find_next_priority();
    	void main_menu_input(char input);
    	void make_job_from_cin();
    	void read_dependencies(Job *j, bool externalFile, const std::istream &inFile);
    	void start_processing(Job *new_process);
    	void lookup_from_input();
    	void update_successors();
    	void process_job();
    	void kill_job();
    	void convert_to_latent(Job *j);
    	void add_from_file();
    	bool make_job_from_line(std::istream &inFile);
    	void output_status(int slice);
    	void complete_processing();
    	void update_stats();	
    	void deep_search_update(Job *j, int num);
};

#endif // __scheduler_h__
	