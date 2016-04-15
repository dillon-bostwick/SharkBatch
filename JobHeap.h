#ifndef __JobHeap_h__
#define __JobHeap_h__

#include <vector>
#include "Job.h"

class Job; //avoids a circular class dependency -- got this from Stack Overflow!

class JobHeap {
	public:
		JobHeap();
		~JobHeap();
		
		void insert(Job *j);

	private:
		
};

#endif // __JobHeap_h__