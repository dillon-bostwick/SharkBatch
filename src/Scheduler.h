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

		//Runtime loop that exists until exit specified by user (or exception thrown)
    	void run();

	private:
		//Constants///////////////////////////////////////////////////////////////////////
		
    	static const int MAX_MEMORY = 1000; // Arbitrary unit -- could be KB
    	static const unsigned long JIFFIE_TIME = 100;
    	//A jiffie is an arbitrary unit of time, and is the minimum unit for which
    	//the CPU must process work. The JIFFIE_TIME constant represents number of
    	//microseconds of wallclock time equivalent to one jiffie of work in realtime
    	//SharkBatch processing
    	
    	//Vars set by constructor and, at this point, cannot be changed during runtime
	    int BASE_QUANTUM;	   //Baseline quantum -- see ReadMe
	    bool VARY_QUANTA;	   //Mode flags -- see ReadMe
		bool CHAIN_WEIGHTING;
		
		//Objects/////////////////////////////////////////////////////////////////////////

		CursesHandler win; //The window which processes all non-fstream I/O
    	
    	std::vector<JobQueue> runs; //A vector of queues of pointers to running jobs

    	JobHashTable jobs; //A hashtable of pointers to all Jobs including those
    					   //that are latent, waiting, running, and completed
    					   
    	JobQueue waitingOnMem; //If a job has no dependencies but there is not enough
    						   //memory available, they wait here (right now it is
    						   //simple FIFO - must pop from the front even if a big
    						   //job is clogging up the queue)
    	
    	//Variables///////////////////////////////////////////////////////////////////////
    	
		int memoryUsed; //Total memory used by all current processes
		
    	//Changes per processor iteration but stored for easy access by methods:
    	Job *current;  //current job being processed
    	int  priority; //current priority of current job
    	bool paused;   //whether processing loop is paused
    	bool exit;     //end the program if true
    	
    	//Used for computing statistics
    	int    runClock; //total jiffies processed since initialization
    	int    totalComplete; //num jobs completed
		int    totalLatency; //
		int    totalTurnaround;
		int    totalResponse;
		double totalTurnPerBurst;
		double totalLatencyPerBurst;
		
		//Methods used for scheduling and processing//////////////////////////////////////
		
		void start_processing(Job *new_process);
		void deep_search_update(Job *j, int num);  
    	void move_from_waiting();
    	bool find_next_priority();
    	void update_successors();
    	void process_job();
    	void complete_processing();
    	
    	//Methods used for IO handling////////////////////////////////////////////////////

    	bool make_job_from_line (std::istream &inFile);
    	bool file_error		    (std::string str, int pid,  std::istream &inFile);
    	int get_dependent_pid   (bool externalFile, int i,  std::istream &inFile);
    	void read_dependencies  (Job *j, bool externalFile, std::istream &inFile);
    	void convert_to_latent  (Job *j);
    	void job_on_console     (Job *j);
    	void kill_check_continue(Job *j);
    	void main_menu_input    (char input);
    	void output_status      (int  slice);
    	void lookup_from_input();
    	void kill_job();
    	void add_from_file();
    	void update_stats();
    	void make_job_from_cin();
    	int  cin_pid();
    	int  cin_exec_time();
    	int  cin_resources();

};

#endif // __scheduler_h__
	