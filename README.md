
-----------------------------------------------------------------------------------------

#SharkBatch
A multilevel feedback queue scheduling algorithm that lets you specify number of queue
priority levels from the command line, and lets you specify job dependencies as you add
jobs on-the-fly.

##Installation & Running
Install:<br>
```$ make```<br>
Run program:<br>
```$ ./sharkbatch -cq baseQuantum numPriorities```<br>
Uninstall:<br>
```$ make clean```<br><br>

##Command line
-c: Chain Weighting Mode (see below)<br>
-q: Varying Quanta Mode (see below)
baseQuantum: Size of quantum (in jiffies) of the baseline priority
numPriorities: number of levels to the multilevel feedback queue (see below)

The idea behind command line arguments is that the user can quickly and easily experiment
with different scheduler criteria and determine how these changes effect the fairness
and efficiency of the scheduler, by running the same dataset and observing the statistics
panel.

For example: finding the right combination between baseline size and number of priorities
can significantly impact how the scheduler handles jobs with medium-length burst times.
With this in mind, enabling Quanta Mode might justify a very large number of priorities
and could even out how the scheduler handles a variety of burst lengths

##Compatibility
I built everything in OS X El Capitan and it works fine; the only problem I've seen is
that the RedHat machines at Tufts EECS aren't compatible with the <thread> and <chrono>
used by Scheduler.cpp to allow sleeping for a quantity of microseconds. I included a
version that works on the Tufts machines but does not sleep, so runtime is a lot faster.
Other than that, everything compiles with both g++ and clang++ on my Mac and at Tufts.

##About the Multilevel Feedback Queue Scheduling Algorithm

Brief introduction to multilevel feedback queues:
https://en.wikipedia.org/wiki/Multilevel_feedback_queue

Good overview of MLFQs, specifically the one that Solaris OS uses:
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf

Some more theoretical stuff about the MLFQ that is worth a skim:
http://dl.acm.org/citation.cfm?doid=321707.321717

In SharkBatch, when jobs are created, a priority is never specified. All jobs start in
the highest priority level queue. The intent is that quick processes (like I/O) get a
chance to run quickly and interrupt longer, batch-like processes, which end up in the
lower level queue for round-robin processing. Of course, it is possible to game the
scheduler and starve the batch-like processes, but overall, the MLFQ is know as a
relatively fair scheduler, although with a higher than average overhead.

##Slice computation via quanta

There are a number of different ways SharkBatch can compute the length of a slice per job.
SharkBatch calls these methods "modes," which must be selected via command line arguments.
First, a baseline quantum must be selected from the command line. If no modes are
selected, all slices are equal to the baseline quantum. Both modes can also be turned on
at the same time (specify -cq):

###Varying Quanta Mode
If varying quanta mode is selected by the -q flag, higher priority jobs receive a shorter
slice. Each priority has a different quantum, which is computed as:

BaseQuantum - priorityLevel (baseQuantum /numPriorities)

Thus the difference between each priority level is constant. This is a commonly used
technique to further enforce the purpose of the MLFQ as longer jobs get pushed to lower
priorities

###Chain Weighting Mode
This is a technique that I invented to experiment with SharkBatch. As far as I am aware,
I have not seen this method used in other scheduling algorithms before. This is most
likely because it makes it very easy for a program to "game" the scheduler and have way
too much control over process starvation. Here is how it works:

If a job A is dependent upon a job B which is dependent upon a job C, then job C has a
"chain" of successors. Job C's longest chain is 2, in this case. If Chain Weighting
is enabled, a job's slice is multiplied by the job's longest chain.

The idea behind this is to increase overall throughput and decrease overall response time.
In practice, only a negligible difference was observed when compared with the same
dataset. I still think this is an interesting idea though and would want to experiment
more with different optimization methods (of course some practical applications might
exist but this might not be ideal for an OS that wants to minimize overhead and obviously
is useless if an OS needs to minimize "gaming").

Note also that at this point, it makes more sense to calculate longest chain based on
total number of jobs, not net expected execution time, because A) the point is to
optimize throughput and response time of all completions in general and B) SharkBatch is
intentionally designed so that a job's execution time does not determine how it will be
prioritized (see below)

Due to my algorithm implementation, the only time that Chain Weighting Mode negatively
effects scheduler overhead is when new jobs are being added. Unfortunately, exponential
complexity does begin to cause a noticeably lag when uploading a file with as little as
300 jobs, however.

##About scheduler statistics:

For a job, the following statistics represent the time elapsed between a job's key events,
and here is how I define them:

Turnaround = Complete - Begin<br>
Latency    = Begin    - Insert<br>
Response   = Complete - Insert

Throughput = # complete jobs / total time elapsed

In SharkBatch, the clock only records time for which a jiffie is being processed;
wallclock time is never recorded.

Here are some statistics I came up with to try and understand the efficiency of the
scheduler:

Avg latency
Avg response time
avg turnaround time
avg turnaround per burst time = mean(turnaround/burst for each job)
Avg latency per burst time = mean(latency/burst for each job)

I have not seen the last two statistics emphasized in any literature so I am not sure
whether they are a valid construct for measuring the scheduler's efficiency, but it
seemed like they would be appropriate considering SharkBatch records the execution time
of all jobs

For a good introduction to scheduler criteria, see
http://www.cs.tufts.edu/comp/111/notes/Scheduling.pdf

Here is an interesting article I referred to when trying to learn about maximizing
throughput:
https://www.cis.upenn.edu/~sudipto/mypapers/throughput.pdf

##About inputting jobs and job dependencies

Sharkbatch allows it's client to input new jobs synchronously between slices of processes.
When adding a job, specify a new PID, expected execution time, memory required, and
dependencies. If a job is dependent on some jobs that haven't been finished, sharkbatch
will hold that job before pushing it to the MLFQ. It is possible to enter a dependent
PID that the scheduler has never seen before.

Note: because SharkBatch is simulating process execution, the user must input an
execution time that represents total CPU burst the job requires. The scheduler does not
use this number to make any decisions regarding time slices or priotizing, making it
possible to remove this variable and instead make jobs block or quit whenever they are
done with the CPU.

##References

##More about job processing

http://www.scs.stanford.edu/07au-cs140/notes/l5.pdf
http://inst.eecs.berkeley.edu/~cs162/sp11/sections/cs162-sp11-section5-answers.pdf
http://www.cs.tufts.edu/comp/111/
https://www.cis.upenn.edu/~sudipto/mypapers/throughput.pdf
https://www.cs.uic.edu/~jbell/CourseNotes/OperatingSystems/5_CPU_Scheduling.html
http://www.personal.kent.edu/~rmuhamma/OpSystems/Myos/cpuScheduling.htm
https://en.wikipedia.org/wiki/Scheduling_(computing)
https://en.wikipedia.org/wiki/Multilevel_feedback_queue
https://en.wikipedia.org/wiki/Fixed-priority_pre-emptive_scheduling

http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf
http://dl.acm.org/citation.cfm?doid=321707.321717

##Thank you

Thank you to Maxwell Bernstein and Erica Schwartz for guiding me through the project over
the last few weeks

