# multi-threaded scheduling simulator. [CS342 Operating Systems - Project2]

Name: Zeynep Cankara
Id: 21703381

The directory content (excluding .h files):
    - Makefile : compiles and links the files
    - schedule.c : producer part of the simple producer-consumer program
    - readyqueue.c : consumer part of the simple producer-consumer program
    - test.c : contains tests for the preemptive algorithms
    - report.pdf : contains the experiment results and implementation details
    - infile-1.txt...infile-5.txt: examples to run the simulator from files

Running the simulator:
	- Find the directory project locates
	- $ make
	- $ ./schedule <N> <Bcount> <minB> <avgB> <minA> <avgA> <ALG>
    - Example:$ ./schedule 375 100 200 1000 1500 FCFS

Reading the CPU burst and interarrival time from a file:
    - $ ./schedule <N> <ALG> -f <inprefix>
    - Example:$ ./schedule 5 FCFS -f infile