/* JobHashTable
 * by Dillon Bostwick
 *
 * A hash table of pointers to jobs. Jobs are hashed by their PID.
 *
 * COLLISION HANDLING:
 * Uses the chaining method. I.e. an array of vectors of pointers to jobs.
 *
 * SPECIFYING SIZE WHEN INITIALIZING:
 * When initializing, the client can choose to manually set the capacity of the table.
 * (As the number of elements in the hash table approaches capacity, the probability
 * that the average case of a lookup is worse than O(1) increases).
 *
 * HASHING:
 * It's easy to change the hash function via the hash(pid) definition. Right now, it is
 * just pid modulo capacity because I assume the user will enter a random range of PIDs, 
 * but the class is designed to allow a more advanced compression if needed. hash(pid)
 * handles wrapping on its own, so if the function is changed, it should always end in
 * modulo capacity.
 *
 * EXPANDING:
 * When the load factor hits .8, the table always doubles in capacity.
 *
 *
 * Scheduler needs to know the PIDs of all completed processses. Currently, when a job is 
 * completed, the job is stored by the program and is not freed from memory until
 * SharkBatch is terminated. For convenience, SharkBatch is implemented such that if
 * memory management becomes a problem in the future, the hash table could store nodes
 * that stores a PID and a pointer to a job, and the job could be freed upon
 * completion (a NULL pointer would indicate the PID is completed).
 */
 
#ifndef __JobHashTable_h__
#define __JobHashTable_h__

#include <vector>
#include "Job.h"

class Job;

//if a job is complete, then the pid is stored but the jobPtr is null

class JobHashTable {
	public:
		 JobHashTable();
		 JobHashTable(int capacity);
		~JobHashTable();
		
		//Lookup a job; if the job doesn't exist, returns NULL
		Job *find(int pid);

		//Push a job pointer
		void insert(Job *j);
		
		//Pop a job from the hash table (the job still exists in the heap)
		bool remove(int pid);
		
		//print all to cout in no order
		void print();
		
		bool is_empty();
		
	private:
		static const int DEFAULT_CAP = 100;
				  double LOAD_FACTOR_THRESHOLD; //= .8 (my C++11 compiler doesnt support
				  								//non-integral const - see constructor)
				  								
		int capacity; //N (number of buckets)
		int size;     //n (number of jobs)
		
		std::vector<Job*> *buckets; //a pointer to an array of vectors of pointers to jobs
				
		int  hash(int pid);
		void expand();
};

#endif //__JobHashTable_h__