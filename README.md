# Multi Threaded Scheduling Simulator

Implementation of a multi threaded scheduling simulator in Linux OS

## Contents

- Makefile : compiles and links the files
- schedule.c : producer part of the simple producer-consumer program
- readyqueue.c : consumer part of the simple producer-consumer program
- test.c : contains tests for the preemptive algorithms
- report.pdf : contains the experiment results and implementation details
- infile-1.txt...infile-5.txt: examples to run the simulator from files

## How to Run

- Generating CPU bursts and interarrival times from exponentially distributed random variable
```
$ make
$ ./schedule <N> <Bcount> <minB> <avgB> <minA> <avgA> <ALG>
```

Example run
```
$ make
$ ./schedule 375 100 200 1000 1500 FCFS
```

- Reading the CPU bursts and interarrival times from a file
```
$ ./schedule <N> <ALG> -f <inprefix>
```

Example run
```
$ ./schedule 5 FCFS -f infile
```