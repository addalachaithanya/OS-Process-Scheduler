1. Objectives

The objectives of this lab are to get familiar with the concepts of process management like process priorities, scheduling, and context switching.

2. Readings

The XINU source code (in sys/), especially those related to process creation (create.c), scheduling (resched.c, resume.c, and suspend.c), termination (kill.c), priority change (chprio.c), as well as other related utility programs (e.g., ready.c) and system initialization code (initialize.c).

3. What to do

You will also be using the csc501-lab0.tgz you have downloaded and compiled by following	the lab	setup guide in PA0, but you need to rename the whole directory to csc501-lab1.

In this lab you will implement two new scheduling policies that will avoid starvation. At the end of this lab you will be able to explain the advantages and disadvantages of the two	new scheduling policies.

The default scheduler in XINU will schedule the process with the highest priority. Starvation occurs in XINU when there are two or more processes eligible for execution that have different priorities. The higher priority process gets to execute first which may result in lower priority processes never getting any CPU time unless the higher priority process ends.

The two scheduling policies that you need to implement, as described below, should address this problem. Note that for each of them, you need to consider how to handle the NULL process, so that the NULL process is selected to run when and only when there are no other ready processes.

For the Linux-like scheduling policy, a valid process priority value is an integer between 0 to 99, where 99 is the highest priority.

1) Linux-like Scheduler (based loosely	on the 2.2 Linux kernel)

This scheduling algorithm tries to loosely emulate the Linux scheduler in 2.2 kernel. In this assignment, we consider all the processes "conventional processes" and uses the policies of the SCHED_OTHER scheduling class within 2.2 kernel. With this algorithm, the CPU time is divided into epochs. In each epoch, every process has a specified time quantum, whose duration is computed at the beginning of the epoch. An epoch will end when all the runnable processes have used up their quantum. If a process has used up its quantum, it will not be scheduled until the next epoch starts, but a process can be selected many times during the epoch if it has not used up its quantum.

When a new epoch starts, the scheduler will recalculate the time quantum of all processes (including blocked ones). This way, a blocked process will start in the epoch when it becomes runnable again. New processes created in the middle of an epoch will wait till the next epoch. For a process that has never executed or has exhausted its time quantum in the previous epoch, its new quantum value is set to its process priority (i.e., quantum = priority). A quantum of 10 allows a process to execute for 10 ticks (10 timer interrupts) within an epoch. For a process that did not get to use up its previously assigned quantum, we allow part of the unused quantum to be carried over to the new epoch. Suppose for each process, a variable counter describes how many ticks are left from its quantum, then at the beginning of the next epoch, quantum = floor(counter/2) + priority. For example, a counter of 5 and a priority of 10 will produce a new quantum value of 12.

During each epoch, runnable processes are scheduled according to their goodness. For processes that have used up their quantum, their goodness value is 0. For other runnable processes, their goodness value is set considering both their priority and the amount of quantum allocation	left: goodness = counter + priority. Again, round-robin is used among processes with equal goodness.

The priority can be changed by explicitly specifying the priority of the	process during the create() system call or through the chprio() function. Priority changes made in the middle of an epoch, however, will only take effect in the next epoch.

An example of how processes should be scheduled under this scheduler is as follows:

If there are processes P1,P2,P3 with priority 10,20,15 then the epoch would be equal to 10+20+15=45 and the possible schedule (with quantum duration specified in the braces) can be: P2(20), P3(15),	P1(10), P2(20), P3(15), P1(10), but not: P2(20), P3(15), P2(20), P1(10).

If you use testmain.c as your main.c program. You are expected to get some results similar to the following: 

MCCCCCCCCCCCCCBBBBBBBMMACCCCCCCCCCCCBBBBBBBMMMA
CCCCCCCCCCCCBBBBBBBMMMACCCCCCCCCCCCCBBBBBBBMMMB
BBBBBBMMABBBBBBBMMMABBBBBBBMMMBMMMAMMAMMMMMMAMM
MAMMMMMAMMMAMMMMMMAMMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA 
2) Multiqueue

This scheduler is similar to the scheduler in part 1, except that it supports two queues: a Real-Time queue and a Normal queue.

You need to add to XINU a modified version of the create() function; name the new function createReal(), which does the same work as create() does, except that the processes it creates are considered as Real-Time processes. Processes created by the original create() function are considered as Normal processes. Real-Time processes go into the Real-Time queue and Normal processes go into the normal queue.

At the start of an epoch, the scheduler generates a random number to decide which queue to schedule in this epoch. It should ensure that in 70% time, the real-time queue is selected and in 30% time, the normal queue is selected.

For processes in the Real-Time queue, the scheduling is round robin. Each process gets a 100-tick quantum. When all runnable processes run up their quantum, the epoch ends.

For processes in the Normal queue, the scheduling algorithm is the same as in part 1.

If a queue is selected but it contains no runnable processes, the scheduler automatically selects the other queue. Again, the NULL process is selected to run when and only when there are no other ready processes in both queues.

Processes created by default (e.g., the master process) are Normal processes.

4. Other implementation details:

1. void setschedclass(int sched_class) 
    This function should change the scheduling type to either of the supplied LINUXSCHED or MULTIQSCHED.

2. int getschedclass() 
    This function should return the scheduling class which should be either LINUXSCHED or MULTIQSCHED.

3. Each of the scheduling class should be defined as constants
    #define LINUXSCHED 1
    #define MULTIQSCHED 2 

4. Some of source files of interest are: create.c, resched.c, resume.c, suspend.c, ready.c, proc.h, kernel.h etc. 
5. Additional	Questions

Write your answers to the following questions in a file named Lab1Answers.txt (in simple text). Please place this file in the sys/ directory and turn it in, along with the above programming assignment:

What are the advantages and disadvantages of each of the two scheduling policies and the original scheduling policy in XINU?


