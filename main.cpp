/*
 * main.cpp
 * by Dillon Bostwick
 *
 * Reads and interprets command line arguments then creates and runs a Scheduler
 */

#include <iostream>
#include <string>
#include "Scheduler.h"

using namespace std;

//Functions helping main
Scheduler *command_line_scheduler_creator(int argc, char *argv[]);
void usageAbort(string program);

//Main creates a Scheduler, calls run(), then deletes the Scheduler.
int main(int argc, char *argv[]) {
	Scheduler *sharkBatch = command_line_scheduler_creator(argc, argv);
	
	sharkBatch->run();
	
	delete sharkBatch;
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

//Given the argc and argv, interpret the command line arguments and allocate a new
//Scheduler object based on the parameters. Notably, we need in total the two boolean
//flags and the two integers required by the Scheduler constructor (see Scheduler.cpp for
//more details)
Scheduler *command_line_scheduler_creator(int argc, char *argv[]) {
	bool chainWeighting = false; //CL flags
	bool varyQuanta = false;
	
	cerr << argc;
	
	if (argc < 3) { //We need the arguments... there is no default
		usageAbort(argv[0]);
	}

	if (argc >= 1 && argv[1][0] == '-') { //Check if a hyphenated flag specifier exists
		if (argc != 4) { //If there are flags we need to check there are more arguments
			usageAbort(argv[0]);
		}
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

	}
	
	//Create a new Scheduler and return a pointer to it
	return new Scheduler(atoi(argv[argc - 2]), atoi(argv[argc - 1]),
						 varyQuanta, chainWeighting);
}

//Output a usage message to cout if the user makes any mistake (or if they are just
//trying to learn how to use the program.
void usageAbort(string program) {
	cout << "Usage: $ " << program << " -cq BASE QUEUENUM" << endl
		 << "-q: Quanta differ such that higher priority queues get shorter slices"<< endl
		 << "-c: \"smart\" slice allocation: A job's slice is multiplied by it's"  << endl
		 << "    longest chain of dependents, allowing important jobs to get extra"<< endl
		 << "    attention and attempting to increase overall throughput" 		   << endl
		 << "BASE: quantum time (in jiffies) given to lowest priority jobs" 	   << endl
		 << "QUEUENUM: number of priority levels (i.e. queues in the MLFQ algorithm"
		 << endl << endl
		 << "For more info see ReadMe and http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-"
		    "sched-mlfq.pdf" << endl;
	exit(1);
}
		 
