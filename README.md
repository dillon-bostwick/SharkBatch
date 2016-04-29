
-----------------------------------------------------------------------------------------

#SharkBatch
A job scheduler simulator in C++.<br><br>

I might describe the algorithm as somewhere between that of the Unix job scheduler (in
terms of queueing) and a more recent cloud task scheduler (in terms of dependency
management, e.g. that of Azure).

##Installation & Running
(Change in to src)<br>
Install:<br>
```$ make```<br>
Run program:<br>
```$ ./sharkbatch -cq baseQuantum numPriorities```<br>
Uninstall (optional):<br>
```$ make clean```<br>

##Command line
-c: Chain Weighting Mode (see below)<br>
-q: Varying Quanta Mode (see below)<br>
baseQuantum: Size of quantum (in jiffies) of the baseline priority<br>
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
I built everything in OS X El Capitan.

Update: Compiler on the Fedora machines still can't include thread and chrono, need to
update with libstdc++, this is only a problem in the Scheduler.cpp file.

Other than that, everything else compiles with both g++ and clang++ in OSX and in Fedora

##The Multilevel Feedback Queue Scheduling Algorithm

In SharkBatch, when jobs are created, a priority is never specified. All jobs start in
the highest priority level queue. The intent is that quick processes (like I/O) get a
chance to run quickly and interrupt longer, batch-like processes, which end up in the
lower level queue for round-robin processing. Of course, it is possible to game the
scheduler and starve the batch-like processes, but overall, the MLFQ is know as a
relatively fair scheduler, although with a higher than average overhead.

The MLFQ algorithm I use is not far off from what Unix uses. I eventually would like to
rewrite in C and see if it can actually be used to run Unix batches.

Background on the MLFQ:<br>
http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf

The Solaris' scheduler is a great example of the sort of algorithm I was going for. It is
straightforward and effective (importantly, the Unix scheduler is not too far off as well
but I like this documentation more):<br>
http://pages.cs.wisc.edu/~remzi/solaris-notes.pdf

Some queue theory stuff regarding the effectiveness of varying quanta, round robin, etc:
<br>
http://dl.acm.org/citation.cfm?doid=321707.321717

##Slice computation via quanta

There are a number of different ways SharkBatch can compute the length of a slice per job.
SharkBatch calls these methods "modes," which can be specified from the command line.
First, a baseline quantum must always be specified as the first numerical argument.<br>

If no modes are selected, all slices are equal to the baseline quantum. Both modes can
also be turned on at the same time, by specifying -cq):

###Varying Quanta Mode
If "varying quanta mode" is selected by the -q flag, higher priority jobs receive a
shorter slice. Each priority has a unique and decreasing quantum:

BaseQuantum - priorityLevel (baseQuantum /numPriorities)

The difference between each priority level is constant. This is a commonly used
technique to further enforce the purpose of the MLFQ as longer jobs get pushed to lower
priorities (see the articles above

###Chain Weighting Mode
This is a technique that I created to experiment with SharkBatch. This is where
SharkBatch can't really be used as an OS scheduler because it requires jobs to have
hard dependencies, a level of determinism the Unix OS doesn't currently have. There are
other problems with it as well, for example, it increasing the probability that a program
can "game" the scheduler and hog CPU time. Here is how it works: <br><br>

If a job A is dependent upon a job B which is dependent upon a job C, then job C has a
"chain" of successors. Job C's longest chain is 2, in this case. If Chain Weighting
is enabled, a job's slice is multiplied by the job's longest chain. <br><br>

The idea behind this is to increase overall throughput and decrease overall response time.
In practice, only a negligible difference was observed when compared with the same
dataset. I still think this is an interesting idea though and would want to experiment
more with different optimization methods. Hopefully SharkBatch could be used as a tool to
quickly get efficiency statistics on different dependency evaluators.  <br>

Microsoft Azure's scheduler is the closest thing I've been able to find to this; it can
take a batch, determine estimated execution times and dependencies (hard or soft), and
optimize the schedule.<br><br>

Due to my algorithm implementation, the only time that Chain Weighting Mode negatively
effects scheduler overhead is when new jobs are being added. Unfortunately, exponential
complexity does begin to cause a noticeably lag when uploading a file with as little as
300 jobs.

##Scheduling statistics reference:

For a job, the following statistics represent the time elapsed between a job's key events,
and here is how I define them:

Turnaround = Complete - Begin<br>
Latency    = Begin    - Insert<br>
Response   = Complete - Insert

Throughput = # complete jobs / total time elapsed

In SharkBatch, the clock only records time for which a jiffie is being processed;
wallclock time is never recorded even if an empty MLFQ is left running.

Here are some statistics I came up with to try and understand the efficiency of the
scheduler:

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

##Memory leaks
Depending on your version of the NCurses library, running valgrind might report some
blocks listed as "still reachable." If you look at the detailed report, this is all due to
NCurses stuff. The only solution I was able to find is to uninstall and reconfigure your
NCurses library........ Bleh.

(Here's why: http://invisible-island.net/ncurses/ncurses.faq.html#config_leaks).

"Any implementation of curses must not free the memory associated with a screen, since
(even after calling endwin()), it must be available for use in the next call to refresh().
There are also chunks of memory held for performance reasons."

##References

####More about job processing

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

####Thank you

Thank you to Maxwell Bernstein and Erica Schwartz at Tufts for guiding me through the
project over the last few weeks

