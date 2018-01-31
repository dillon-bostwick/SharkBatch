
## Install
(```cd `src```)<br>
Install:<br>
```$ make```<br>
Run:<br>
```$ ./sharkbatch```
or, e.g.:
```$ ./sharkbatch -cq <baseQuantum> <numPriorities>```<br>
Uninstall:<br>
```$ make clean```<br>

## Options
```-c```: Chain Weighting Mode (see below)<br>
```-q```: Varying Quanta Mode (see below)<br>
```baseQuantum```: Size of [quantum](https://en.wikipedia.org/wiki/Preemption_(computing)#Time_slice) of the baseline priority in [jiffies](http://man7.org/linux/man-pages/man7/time.7.html)<br>
```numPriorities```: number of levels to the multilevel feedback queue (see below)

## The Multilevel Feedback Queue Scheduling Algorithm

In SharkBatch, when jobs are created, the client does not specify the priority. All jobs start in the highest priority level queue. The intent is that short processes get a chance to run quickly and interrupt longer, batch-like processes, which end up in the lower level queue for round-robin processing. Overall, the [MLFQ is often described as a "relatively fair scheduler."](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf)

More MLFQ reading:<br>

The [Solaris' scheduler](http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf) is a great example of an earlier implementation that was met with success.<br>

More [Queueing theory](http://dl.acm.org/citation.cfm?doid=321707.321717) with some mathematical proofs behind the effectiveness of varying quanta, benefits over round-robin, etc.
<br>

## Slice computation via quanta

SharkBatch can compute the length of a slice per job using an number of different methods called, "modes," which can be specified as flags. (A baseline quantum must always be specified as the first numerical argument).<br>

If no modes are selected, all slices are equal to the baseline quantum. Modes may be simultaneously activated by specifying ```-cq```):

### Varying Quanta Mode
If "varying quanta mode" is selected by the -q flag, then higher priority jobs will receive shorter slices. In this case every priority has a unique, decreasing quantum:

BaseQuantum - priorityLevel (baseQuantum / numPriorities)

The difference between each priority level is constant. This is commonly used to further increase the MLFQ's effectiveness as a wait time discriminator, as longer jobs get pushed to lower priorities.

## Scheduling statistics reference

For a job, the following statistics represent the time elapsed between a job's key events:

Turnaround = Complete - Begin<br>
Latency    = Begin    - Insert<br>
Response   = Complete - Insert

Throughput = # complete jobs / total time elapsed

In SharkBatch, the clock only records time for which a jiffie is being processed;
wallclock time is never recorded even if an empty MLFQ is left running.

Avg latency<br>
Avg response time<br>
Avg turnaround time<br>
Avg turnaround per burst time = mean(turnaround/burst for each job)<br>
Avg latency per burst time = mean(latency/burst for each job)

## Dependency resolution
SharkBatch also supports dependency resolution of jobs. A topological sort will be applied if a client specifies job dependencies into a DAG. Jobs with longer total DAG time will be prioritized in a way consistent with optimizing the entire batch of jobs, however latency of each individual job is balanced with ability to unblock jobs that may be more recent and this have a lower latency expectation.

## About inputting jobs and job dependencies

New jobs are added synchronously between slices of processes. When adding a job, specify 
a new PID, expected execution time, memory required, and dependent jobs. If a job is
dependent on some jobs that haven't been finished, SharkBatch will hold that job before
pushing it to the MLFQ. It is possible to enter a dependent PID that the scheduler has
never seen before.

Note: because SharkBatch is simulating process execution, the user must input an
execution time that represents total CPU burst the job requires. The scheduler does not
use this number to make any decisions regarding time slices or prioritizing, making it
possible to remove this variable and instead make jobs block or quit whenever they are
done with the CPU.

## Known issues
- For earlier versins of NCurses version, when running valgrind memory profile, some mem blocks listed as "still reachable." Ncurses >=6.0 is recommended.

- Update: Can't include thread and chrono in Scheduler for the latest clang 3.9 (tested on Fedora 24), need to
update with libstdc++.

- It is possible to game the scheduler and starve the batch-like processes.

## References
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
