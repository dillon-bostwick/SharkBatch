#ifndef __JobHashTable_h__
#define __JobHashTable_h__

#include "Job.h"

//if a job is complete, then the pid is stored but the jobPtr is null


class JobHashTable {
	
	public:
		JobHashTable();
		~JobHashTable();
				
		void insert(Job *job);
		Job *find_job(int pid);
		void make_complete(int pid);
		void add_successors(Job *job, IntBST *dependencies);
		
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
};

#endif //__JobHashTable_h__