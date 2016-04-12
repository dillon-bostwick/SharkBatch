#ifndef __scheduler_h__
#define __scheduler_h__



class Scheduler {
public:
	Scheduler();
	~Scheduler();
	Scheduler(const Scheduler &source);
    Scheduler &operator= (const Scheduler &source);
    
    add_job(Job *j);
    kill(int pid);
    kill(Job *j);
    
	
private:
	PriorityQueue jobs;

};

#endif // __hand_h__
