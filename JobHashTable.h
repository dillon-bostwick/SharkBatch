/* JobHashTable: A hash table of pointers to jobs. Jobs are hashed by their PID.
 *
 * COLLISION HANDLING:
 * Uses the chaining method. An array of vectors of pointers to jobs.
 *
 * SPECIFYING SIZE WHEN INITIALIZING:
 * When initializing, the client has the chance to set the capacity of the table.
 * As the number of elements in the hash table approaches capacity, the probability
 * that the average case of a lookup is worse than O(1) increases.
 *
 * HASHING:
 * It's easy to change the hash function via the hash(pid) definition. It is set to
 * 							Hash(PID) = PID mod capacity
 * at the moment...
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
		int capacity; //size of the table
		
		std::vector<Job*> *table; //a pointer to an array of vectors of pointers to jobs
				
		int hash(int pid); //The hash function
};

#endif //__JobHashTable_h__