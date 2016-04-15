#ifndef __Scheduler_h__
#define __Scheduler_h__

#include <queue>
#include <vector>
#include "Job.h"
#include "IntBST.h"
#include "JobHashTable.h"

class Scheduler {
	public:
		Scheduler(int numQueues); //numQueues must be non-negative
		~Scheduler();

    	void process();
    	
	private:
	
		//constants
	    static const int BASE_QUANTUM = 1000; //ms
    	static const int DIFF_QUANTUM = 100; //ms
    	static const int MAX_MEMORY = 1000; //KB
    	
    	
    	//Storage
    	std::vector<std::queue<Job*> > runs; //A vector of queues of pointers to Jobs
    	std::vector<int> quants;    //easy access to all the quantums per priority
    	
    	JobHashTable jobs; //A hashtable of pointers to all Jobs including those
    					   //that are waiting, running, and completed
    					   
    	std::queue<Job*> waitingOnMem;
		int memoryUsed;

    	
    	//These are changed for each processor iteration but are stored in private so they
    	//can be accessed everywhere
    	Job *current;
    	int priority; //current priority of current job
    	
    	//private functions and helpers
    	void print_status();
    	bool user_input();
    	void make_job_from_input();
    	IntBST *read_dependencies_and_age();
    	void start_processing(Job *new_process);
    	void lookup_from_input();
    	void update_successors();
};

#endif // __scheduler_h__
	