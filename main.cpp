/*
 * By Dillon Bostwick for Tufts University
 */

#include <iostream>
#include <string>
#include "Scheduler.h"

using namespace std;

int processCommandLine(int argc, char *argv[]);
void usageAbort(string program);



int main(int argc, char *argv[]) {
	int numQueues = processCommandLine(argc, argv);

	Scheduler sharkBatch(numQueues);
	
	sharkBatch.run();
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

int processCommandLine(int argc, char *argv[]) {
	//check one CL argument exists
	if (argc != 2) {usageAbort(argv[0]);}
	
	int numQueues = atoi(argv[1]);
	
	if (numQueues < 1) {usageAbort(argv[0]);}
	
	return numQueues;
}

void usageAbort(string program) {
	cout << "Usage: $ " << program << " numberOfQueues" << endl
		 << "where numberOfQueues is a positive integer specifying levels of MLFQ"
		    "priorities. For more info see http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-"
		    "sched-mlfq.pdf" << endl;
	exit(1);
}