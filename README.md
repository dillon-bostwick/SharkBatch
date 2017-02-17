
##Install
(cd to src)<br>
Install:<br>
```$ make```<br>
Run program:<br>
```$ ./sharkbatch -cq baseQuantum numPriorities```<br>
Uninstall (optional):<br>
```$ make clean```<br>

##CLI
-c: Chain Weighting Mode (see below)<br>
-q: Varying Quanta Mode (see below)<br>
baseQuantum: Size of quantum (in jiffies) of the baseline priority<br>
numPriorities: number of levels to the multilevel feedback queue (see below)

##Compatibility
Update: Can't include thread and chrono for the latest clang on Fedora, need to
update with libstdc++, this is only a problem in the Scheduler.cpp file.

##The Multilevel Feedback Queue Scheduling Algorithm

In SharkBatch, when jobs are created, a priority is never specified. All jobs start in
the highest priority level queue. The intent is that short processes get a
chance to run quickly and interrupt longer, batch-like processes, which end up in the
lower level queue for round-robin processing. Of course, it is possible to game the
scheduler and starve the batch-like processes, but overall, the MLFQ is know as a
relatively fair scheduler.

Background on the MLFQ:<br>
http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf

The Solaris' scheduler is a great example of the algorithm. It is
straightforward and effective:<br>
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf

Theory on effectiveness of varying quanta, round robin, etc:
<br>
http://dl.acm.org/citation.cfm?doid=321707.321717

##Slice computation via quanta

There are a number of different ways SharkBatch can compute the length of a slice per job called modes, which can be specified from the CLI. First, a baseline quantum must always be specified as the first numerical argument.<br>

If no modes are selected, all slices are equal to the baseline quantum. Both modes can
also be turned on at the same time, by specifying -cq):

###Varying Quanta Mode
If "varying quanta mode" is selected by the -q flag, higher priority jobs receive a
shorter slice. Each priority has a unique and decreasing quantum:

BaseQuantum - priorityLevel (baseQuantum /numPriorities)

The difference between each priority level is constant. This is commonly used to further enforce the MLFQ as longer jobs get pushed to lower
priorities (see the articles above


##Scheduling statistics reference

For a job, the following statistics represent the time elapsed between a job's key events:

Turnaround = Complete - Begin<br>
Latency    = Begin    - Insert<br>
Response   = Complete - Insert

Throughput = # complete jobs / total time elapsed

In SharkBatch, the clock only records time for which a jiffie is being processed;
wallclock time is never recorded even if an empty MLFQ is left running.

Avg latency<br>
Avg response time<br>
avg turnaround time<br>
avg turnaround per burst time = mean(turnaround/burst for each job)<br>
Avg latency per burst time = mean(latency/burst for each job)

##About inputting jobs and job dependencies

New jobs are added synchronously between slices of processes. When adding a job, specify 
a new PID, expected execution time, memory required, and dependencies. If a job is
dependent on some jobs that haven't been finished, SharkBatch will hold that job before
pushing it to the MLFQ. It is possible to enter a dependent PID that the scheduler has
never seen before.

Note: because SharkBatch is simulating process execution, the user must input an
execution time that represents total CPU burst the job requires. The scheduler does not
use this number to make any decisions regarding time slices or prioritizing, making it
possible to remove this variable and instead make jobs block or quit whenever they are
done with the CPU.

##Known issues
On some NCurses versions, valgrind might report some mem 
blocks listed as "still reachable."

##References
http://www.scs.stanford.edu/07au-cs140/notes/l5.pdf<br> 
http://inst.eecs.berkeley.edu/~cs162/sp11/sections/cs162-sp11-section5-answers.pdf<br>
http://www.cs.tufts.edu/comp/111/<br>
https://www.cis.upenn.edu/~sudipto/mypapers/throughput.pdf<br>
https://www.cs.uic.edu/~jbell/CourseNotes/OperatingSystems/5_CPU_Scheduling.html<br>
http://www.personal.kent.edu/~rmuhamma/OpSystems/Myos/cpuScheduling.htm<br>
https://en.wikipedia.org/wiki/Scheduling_(computing)<br>
https://en.wikipedia.org/wiki/Multilevel_feedback_queue<br>
https://en.wikipedia.org/wiki/Fixed-priority_pre-emptive_scheduling<br>
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf<br>
http://dl.acm.org/citation.cfm?doid=321707.321717
