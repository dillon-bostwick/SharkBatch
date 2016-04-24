/*
 * By Dillon Bostwick for Tufts University
 */

#include <iostream>
#include <string>
#include "Scheduler.h"

using namespace std;

Scheduler *command_line_scheduler_creator(int argc, char *argv[]);
void usageAbort(string program);

int main(int argc, char *argv[]) {
	Scheduler *sharkBatch = command_line_scheduler_creator(argc, argv);
	
	sharkBatch->run();
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

Scheduler *command_line_scheduler_creator(int argc, char *argv[]) {
	//flags
	bool chainWeighting = false;
	bool varyQuanta = false;
	
	cerr << argc;
	if (argc < 3) {
		usageAbort(argv[0]);
	}

	if (argc >= 1 && argv[1][0] == '-') { //Check if a hyphenated flag specifier exists
		for (int j = 1; argv[1][j] != '\0'; j++) { //interpret the flags
			switch (argv[1][j]) {
				case 'c':
					chainWeighting = true;
					break;
				case 'q':
					varyQuanta = true;
					break;
				default:
					usageAbort(argv[0]);
			}
		}
		
		if (argc != 4) {
			usageAbort(argv[0]);
		}
	}

	return new Scheduler(atoi(argv[argc - 2]), atoi(argv[argc - 1]), varyQuanta, chainWeighting);
}

void usageAbort(string program) {
	cout << "Usage: $ " << program << " -cq BASE QUEUENUM" << endl
		 << "-q: Quanta differ such that higher priority queues get shorter slices" << endl
		 << "-c: \"smart\" slice allocation: A job's slice is multiplied by it's" << endl
		 << "    longest chain of dependents, allowing important jobs to get extra" << endl
		 << "    attention and attempting to increase overall throughput" << endl
		 << "BASE: quantum time (in jiffies) given to lowest priority jobs" << endl
		 << "QUEUENUM: number of priority levels (i.e. queues in the MLFQ algorithm"
		 << endl << endl
		 << "For more info see ReadMe and http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-"
		    "sched-mlfq.pdf" << endl;
	exit(1);
}
		 
