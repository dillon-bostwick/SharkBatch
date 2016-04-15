###FOR MAX & ERICA:

-Can you check my Makefile to just make sure that all the .o are correct? It "works" but 
I don't want to be missing some compiler errors or something

-----------------------------------------------------------------------------------------

#SharkBatch
A multilevel feedback queue scheduling algorithm that lets you specify number of queue
priority levels from the command line, and lets you specify job dependencies as you add
jobs on-the-fly.

##Install & Run
Installation:
$ make
Run program:
$ ./sharkbatch numberOfQueues<br>
Where numberOfQueues is a positive integer less than BASE_QUANTUM/DIFF_QUANTUM, where
BASE_QUANTUM is the time quantum of the lowest level queue priority, and DIFF_QUANTUM
is the difference between each adjacent queue priority (you can't go above base/diff
number of priorities or else you'd get non-positive time slices). Go ahead and experiment
with the "fairness" of the scheduler directly from the command line!!

##About the Multilevel Feedback Queue Scheduling Algorithm

Brief introduction to Multilevel Feedback Queues (MLFQ):
https://en.wikipedia.org/wiki/Multilevel_feedback_queue

Very well-written overview of MLFQs, specifically the one that the Solaris OS uses:
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf

Some rather dense literature that will likely leave none of your questions unanswered:
http://dl.acm.org/citation.cfm?doid=321707.321717

##About inputting jobs and job dependencies
Sharkbatch allows it's client to input new jobs synchronously between slices of processes.
When adding a job, specify a new PID, expected execution time, memory required, and
dependencies. If a job is dependent on some jobs that haven't been finished, sharkbatch
will hold that job before pushing it to the MLFQ. IF SHARKBATCH DETERMINES THAT A LOW
PRIORITY JOB IS VITAL TO THE INITIATION OF MANY OTHER JOBS AND IS CLOGGING THE WAITING
QUEUE, SHARKBATCH WILL AUTOMATICALLY AGE THAT JOB TO THE HIGHEST PRIORITY LEVEL IN
ORDER TO GIVE IT AN ADVANTAGE OVER THE CPU.*

*still working on this. But from what I've read so far, I haven't found any MLFQ
schedulers that do this - I think most of them age all processes at the same time on an
arbitrary interval. This might be because my algorithm greatly increases the ability
of an application to "game" the scheduler and maliciously monopolize the CPU.