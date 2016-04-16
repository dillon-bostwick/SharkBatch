#ifndef __JobHashTable_h__
#define __JobHashTable_h__

#include "Job.h"

//if a job is complete, then the pid is stored but the jobPtr is null


class JobHashTable {
	
	public:
		JobHashTable();
		~JobHashTable();
				

		Job *find_or_insert(int pid); //searches for a job, and if the job doesn't exist,
									  //automatically insert the job
		Job *find(int pid); //will only search for the job and throws exception if not found
		void make_complete(int pid);
		void add_successors(Job *job, IntBST *dependencies);
		void premature_kill(int pid);
		
	private:

			
		static const int SIZE = 100;
		
		struct Node {
			int pid;
			Job *jobPtr; //==NULL means the job is complete
			Node *next;  //==NULL means end of the chain
		};
				
		Node *table[SIZE]; //an array of pointers to nodes (which point to jobs)
		
		//helpers
		int hash(int pid);
		void remove(Node *node);
		Node *find(int pid, Node *node); //NB that this returns a node, not job
		Node *find_or_insert(int pid, Node *node);
		Node *insert(int pid);
};

#endif //__JobHashTable_h__