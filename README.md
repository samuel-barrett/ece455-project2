# ece455-project2
 
https://docs.google.com/document/d/1QU-0ev9iSK0MUagf5oXr_Ua0lOKG8e4KOBG8s2ef-X4/edit?usp=sharing

Project 2 Design Document
ECE 455 

-----------------------
Tasks
-----------------------
We will use 3 permanent tasks in our system which will be used to schedule and record statistics about “User Tasks”. User Tasks are those that the EDF scheduler schedules, and are decided by the user. 

-----------------------
DD-Scheduler
-----------------------
The DD_scheduler will be implemented as a high priority RTOS task, that will be triggered upon receiving values from a a queue, and will thus generate 

We will use messages in this task, to receive values from the 

It will use a timer for user task generation 

The DD schdeduler will dynamically change the priority of USER RTOS projects so that only one task (that which the scheduler wants to run) will be running at a time

------------------------
DD-Task Generator
------------------------

The deadline driven task scheduler will be triggered be an interrupt service routine (ISR) that is called every time the timer expires. We will create three different timer, one for each of the three tasks used
in our test bench.

------------------------
Monitor Task
------------------------

The monitor task will collect statistics about completed tasks and overdue tasks, printing them to the console when running

------------------------
Lists
------------------------

We will use three linked. An active task list queue, and overdue task list queue, and a completed task list queue.

The list interface must include a sort feature, to sort tasks by earliest deadline.

