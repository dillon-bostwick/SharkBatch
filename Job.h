#ifndef __job_h__
#define __job_h__

#include <string>
#include <vector>
#include "JobHashTable.h"

class Job; //forward declaration prevents circular reference in typedef below
class JobHashTable;

/* LATENT: The job has been referenced by the dependencies of some other job, so
 * 			the PID and at least one successor has been recorded, but no other
 *			metadata exists because the job has not actually been added by client
 * WAITING: The job has been officially added as a future processes with metadata,
 *			but is either waiting on more memory or on dependencies.
 * RUNNING: The job is now a processes in the multilevel feedback queues
 * COMPLETE: The job's execution time has hit 0. It will be freed from the heap.
 */
enum Status {LATENT, WAITING, RUNNING, COMPLETE};

typedef std::vector<Job*> JobList; //injecting this everywhere

class Job {		
	public:
		 Job(int pid); //creates a latent job
		~Job();
		
		void prepare(int execTime, int resources); //turns a latent job into a waiting job



		//get stuff
		int      get_pid();
		int      get_exec_time();
		int      get_resources();		
		int      get_latency();
		int      get_response();
		int      get_turnaround();	
		int      get_original_exec();
		int		 get_deep_success();
		JobList *get_successors();
		JobList *get_dependencies();
		Status   get_status();
		int 	get_longest_chain();

		//set stuff
		int  decrease_time(int time);
		void add_dependency(Job *j);
		void add_successor(Job *j);
		void remove_dependency(int pid);
		void set_status(Status status);
		void set_clock_insert(int time);
		void set_clock_begin(int time);
		void set_clock_complete(int time);
		void set_longest_chain(int num);

		//determine stuff
		bool no_dependencies();
		bool no_successors();

		//print stuff to cout for testing
		void print_successors();
		void print_dependencies();
		
	private:
		//Job metadata
		int pid;
		int execTime;
		int resources;
		int longestSuccesschain;
		Status status;
		
		//Used for statistics
		int originalExecTime;
		int clockInsert;
		int clockBegin;
		int clockComplete;
		
		JobList dependencies; //pointers to jobs that need to finished before
						    //this one can start. The scheduler checks whether this
						    //is empty before it pulls it from the JobHeap which
						    //includes all the "waiting" jobs.
		JobList successors; //A vector of pointers to jobs that are dependent upon
							//this job. When this job is completed, the scheduler
							//will find all the jobs on this list, find take this
							//job's PID, and remove it from the successor's dependent
							//list. Note that order is important, because higher up
							//jobs were added earlier and thus should be the first
							//to enter the MLFQ in the case that the completion of
							//this job causes more than one successor to be able
							//to begin processing
};

#endif // __job_h__