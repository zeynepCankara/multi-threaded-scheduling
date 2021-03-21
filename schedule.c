/**
 * A multi-threaded scheduling simulator.
 * Runs N threads concurrently  and generating cpu  bursts  (workload).
 * A server threads handles the scheduling and execution of the bursts.
 * @author Zeynep Cankara - 21703381
 * @version 1.0
 **/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "readyqueue.h"

#define MAX_THREADS 10 // maximum number of threads

// global variables
int N;
int Bcount;
int minB;
int avgB;
int minA;
int avgA;
char *alg;
char *inprefix;
struct readyqueue *rq;

// argument list for the thread
typedef struct argvThread
{
    int t_index; // thread id
    pthread_attr_t t_attr;
    pthread_cond_t t_cond; // condition variable
} argvThread;

struct argvThread t_args[MAX_THREADS]; // pass the thread arguments
pthread_cond_t t_cond_wait = PTHREAD_COND_INITIALIZER;

static void *do_task(void *arg_ptr)
{
    int t_id = ((struct argvThread *)arg_ptr)->t_index;
    pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;
    printf("thread %d started\n", t_id + 1);

    // perform task...
    for (int b_index = 0; b_index < Bcount; b_index++)
    {
        // Get Burst Duration
        int burstTime = getBurstLength(NULL);
        // Send burst duration to queue
        pushBurst(rq, t_id, b_index, burstTime);
        pthread_cond_signal(&t_cond_wait);
        pthread_cond_wait(&(t_args[t_id].t_cond), &t_mutex); // wait cond mutex
        // Get Sleep Duration
        int sleepTime = getInterarrivalLength(NULL);
        usleep(sleepTime * 1000); // in ms
    }

    pushBurst(rq, t_id, Bcount, -1);
    pthread_cond_signal(&t_cond_wait);
    pthread_exit(NULL);
}

// generates random exponential number from mean
double generateRandomExpNum(int mean)
{
    if (mean == 0)
    {
        printf("ERROR: undefiened lambda parameter");
        exit(0);
    }
    double lambda = 1 / (double)(mean);
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -1 * log(1 - u) / lambda;
}

// time for CPU burst length and wait time
double getRandExpTime(int mean, int lowerLimit)
{
    double randExpNum = generateRandomExpNum(mean);
    while (randExpNum < lowerLimit)
    {
        randExpNum = generateRandomExpNum(mean);
    }
    return randExpNum;
}

int getBurstLength(FILE *fp)
{
    int length = 0;
    if (fp == NULL)
    {
        length = getRandExpTime(avgB, minB);
    }
    else
    {
        // READ burst time from file and assign it to burstTime
        fscanf(fp, "%*s %d", &length);
    }
    return length;
}

int getInterarrivalLength(FILE *fp)
{
    int length = 0;
    if (fp == NULL)
    {
        length = getRandExpTime(avgA, minA);
    }
    else
    {
        // READ burst time from file and assign it to burstTime
        fscanf(fp, "%*s %d", &length);
    }
    return length;
}

// Test funtions

void fcfsTest(struct readyqueue *rq)
{
    pushBurst(rq, 1, 1, 3);
    pushBurst(rq, 2, 2, 4);
    pushBurst(rq, 3, 3, 5);
    printReadyqueue(rq->head);
    printf("(1, 2, 3) printed \n");
    struct burst *b = fcfs(rq);
    pushBurst(rq, 3, 4, 5);
    pushBurst(rq, 4, 5, 6);
    printReadyqueue(rq->head);
    printf("(2, 3, 4, 5) printed \n");
    b = fcfs(rq);
    b = fcfs(rq);
    pushBurst(rq, 4, 6, 6);
    pushBurst(rq, 4, 7, 6);
    printReadyqueue(rq->head);
    printf("(4, 5, 6, 7) printed \n");
    deleteReadyqueue(rq->head);
}

void sjfTest(struct readyqueue *rq)
{
    pushBurst(rq, 1, 8, 8);
    pushBurst(rq, 1, 7, 7);
    pushBurst(rq, 1, 9, 9);
    printReadyqueue(rq->head);
    printf("(8, 7, 9) printed \n");
    struct burst *b = sjf(rq);
    pushBurst(rq, 1, 3, 3);
    pushBurst(rq, 1, 5, 5);
    printReadyqueue(rq->head);
    printf("(8, 9, 3, 5) printed \n");
    b = sjf(rq);
    b = sjf(rq);
    pushBurst(rq, 1, 2, 2);
    pushBurst(rq, 1, 1, 1);
    printReadyqueue(rq->head);
    printf("(8, 9, 2, 1) printed \n");
    deleteReadyqueue(rq->head);
}

void getRandExpTimeTest(int mean, int lowerLimit)
{
    int CNT_RANDOM_NUM = 10;
    for (int i = 0; i < CNT_RANDOM_NUM; i++)
    {
        double randNum = getRandExpTime(mean, lowerLimit);
        printf("generated random val: %f \n", randNum);
    }
}

// Main Program
int main(int argc, char *argv[])
{
    int isFromFile = 0;
    if (argc == 8)
    {
        // read CPU workload information from terminal
        N = atoi(argv[1]);
        Bcount = atoi(argv[2]);
        minB = atoi(argv[3]);
        avgB = atoi(argv[4]);
        minA = atoi(argv[5]);
        avgA = atoi(argv[6]);
        alg = argv[7];
        printf(" argv{N: %d, Bcount: %d, minB: %d, avgB: %d, minA: %d, avgA: %d, alg: %s}\n", N, Bcount, minB, avgB, minA, avgA, alg);
    }
    else if (argc == 5)
    {
        // read CPU workload information from file
        isFromFile = 1;
        N = atoi(argv[1]);
        alg = argv[2];
        if (strcmp(argv[3], "-f") != 0)
        {
            printf("Error: Flag does not exist. Only supported -f to read from file. \n");
            printf("Example use:$ schedule <N> <ALG> -f <inprefix>\n");
            return -1;
        }
        inprefix = argv[4];
        printf("argv{N: %d, alg: %s, inprefix: %s}\n", N, alg, inprefix);
    }
    else
    {
        printf("Error: Number of arguments does not match with any mode.\n");
        printf("read from terminal mode:$ schedule <N> <Bcount> <minB> <avgB> <minA> <avgA> <ALG>\n");
        printf("read from file mode:$ schedule <N> <ALG> -f <inprefix>\n");
        return -1;
    }

    // create the readyqueue
    rq = initReadyQueue();

    // create the threads
    pthread_t tids[MAX_THREADS]; // thread ids

    int ret;

    for (int i = 0; i < N; ++i)
    {
        t_args[i].t_index = i;
        pthread_attr_init(&t_args[i].t_attr);
        t_args[i].t_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

        ret = pthread_create(&(tids[i]),
                             &t_args[i].t_attr, do_task, (void *)&(t_args[i]));

        if (ret != 0)
        {
            printf("ERROR: thread create failed \n");
            exit(1);
        }
        printf("thread %i with tid %u created\n", i,
               (unsigned int)tids[i]);
    }

    printf("main: waiting all threads to terminate\n");
    for (int i = 0; i < N; ++i)
    {
        ret = pthread_join(tids[i], NULL);
        if (ret != 0)
        {
            printf("thread join failed \n");
            exit(0);
        }
    }

    printf("main: all threads terminated\n");

    return 0;
}