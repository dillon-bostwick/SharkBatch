#ifndef __job_h__
#define __job_h__



class Hand {
public:
	Job();
	Job(int priority, int execTime, int resources, bool canKill);
	~Job();
	Job(const Job &source);
  Job &operator= (const Job &source);
    
  //setters and getters..........
	
private:
	string name;
	int pid;
	int timeIn;
	int priority;
	int execTime; //infinity?
	int timeElapsed;
	int resources;
	bool canKill;
	DynamicArray dependent; //PIDs of all the dependencies
};

#endif // __hand_h__
