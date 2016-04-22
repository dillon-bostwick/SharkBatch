
-----------------------------------------------------------------------------------------

#SharkBatch
A multilevel feedback queue scheduling algorithm that lets you specify number of queue
priority levels from the command line, and lets you specify job dependencies as you add
jobs on-the-fly.

##Installation & Running
Install:<br>
```$ make```<br>
Run program:<br>
```$ ./sharkbatch numberOfQueues```<br>
Uninstall:<br>
```$ make clean```<br><br>
Where numberOfQueues is a positive integer less than BASE_QUANTUM/DIFF_QUANTUM, where
BASE_QUANTUM is the time quantum of the lowest level queue priority, and DIFF_QUANTUM
is the difference between each adjacent queue priority (you can't go above base/diff
number of priorities or else you'd get non-positive time slices). The point is,
you can experiment with the "fairness" of the scheduler straight from the command
line!

##About the Multilevel Feedback Queue Scheduling Algorithm

Brief introduction to Multilevel Feedback Queues (MLFQ):
https://en.wikipedia.org/wiki/Multilevel_feedback_queue

Good overview of MLFQs, specifically the one that Solaris OS uses:
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf

Some more theoretical stuff about the MLFQ that is worth a skim:
http://dl.acm.org/citation.cfm?doid=321707.321717



-Is pre-emptive
-My scheduler jobs aren't truly "bursts", although my stats bar refers to them that way,
 so I call them execTime in the code

###About scheduler statistics:

/////////////TODO

Here's a paper about throughput maximization of different algorithms:
https://www.cis.upenn.edu/~sudipto/mypapers/throughput.pdf

##About inputting jobs and job dependencies
Sharkbatch allows it's client to input new jobs synchronously between slices of processes.
When adding a job, specify a new PID, expected execution time, memory required, and
dependencies. If a job is dependent on some jobs that haven't been finished, sharkbatch
will hold that job before pushing it to the MLFQ. You can even enter a dependent
PID that the scheduler has never seen before.

I am currently working on a way for SharkBatch to determine if some low priority
job is vital to some important dependency chain of other jobs, and will detect
this starvation automatically and age the job to high priorities. So far, I am not
aware of any OS that does this -- some of them age all jobs depending on some
arbitrary iteration. Maybe the reason they don't do this is because it would make
it very easy for an application to "game" the scheduler and monopolize the CPU
by repeatedly sending dependent jobs. But even then, there are still other ways
to "game" a MLFQ anyway!












http://www.scs.stanford.edu/07au-cs140/notes/l5.pdf
http://inst.eecs.berkeley.edu/~cs162/sp11/sections/cs162-sp11-section5-answers.pdf
http://www.cs.tufts.edu/comp/111/
https://www.cis.upenn.edu/~sudipto/mypapers/throughput.pdf
https://www.cs.uic.edu/~jbell/CourseNotes/OperatingSystems/5_CPU_Scheduling.html
http://www.personal.kent.edu/~rmuhamma/OpSystems/Myos/cpuScheduling.htm
https://en.wikipedia.org/wiki/Scheduling_(computing)
https://en.wikipedia.org/wiki/Multilevel_feedback_queue
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf
http://dl.acm.org/citation.cfm?doid=321707.321717




