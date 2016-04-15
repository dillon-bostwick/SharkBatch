#ifndef __job_h__
#define __job_h__

#include <string>
#include <vector>
#include "IntBST.h"

class Job;
typedef std::vector<Job*> JobList; //injecting this everywhere
enum Status {NOTAJOB, WAITING, RUNNING, COMPLETE};

class Job {		
	public:

		
		Job(int pid, int execTime, int resources, IntBST *dependencies);
		Job(int pid);
		
		void decrement_time();
		bool is_complete();
		int get_pid();
		int get_exec_time();
		int get_resources();
		void set_status(Status status);
		Status get_status();
		void add_successor(Job *j);
		void remove_dependency(int pid);
		void print_successors();
		void print_dependencies();
		Job *get_successor(int index);
		int get_successor_size();
		void remove_pid_from_dependencies(int pid);
		bool no_dependencies();
		void initialize_job(int execTime, int resources, IntBST *dependencies);
		
	private:
		int pid;
		int execTime;
		int resources;
		Status status;
		bool age;
		
		IntBST *dependencies; //a IntBST of PIDs of jobs that need to finished before
						   //this one can start. The scheduler checks whether this
						   //is empty before it pulls it from the JobHeap which
						   //includes all the "waiting" jobs
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