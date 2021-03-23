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
#include <sys/time.h>

#include "readyqueue.h"
#include "test.h"

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

// for thread management
pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

// argument list for the thread
typedef struct argvThread
{
    int t_index; // thread id
    pthread_t tid;
    pthread_attr_t t_attr;
    pthread_cond_t t_cond; // condition variable
} argvThread;

struct argvThread t_args[MAX_THREADS]; // pass the thread arguments
pthread_cond_t t_cond_wait = PTHREAD_COND_INITIALIZER;

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

void getRandExpTimeTest(int mean, int lowerLimit)
{
    int CNT_RANDOM_NUM = 10;
    for (int i = 0; i < CNT_RANDOM_NUM; i++)
    {
        double randNum = getRandExpTime(mean, lowerLimit);
        printf("generated random val: %f \n", randNum);
    }
}

static void *do_task(void *arg_ptr)
{
    int t_id = ((struct argvThread *)arg_ptr)->t_index;
    pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;
    printf("thread %d started\n", t_id + 1);

    // perform task...
    int b_index = 0;
    while (b_index < Bcount)
    {
        // Get Burst Duration
        int burstTime = getBurstLength(NULL);
        printf(" (do_task)-burstTime: %d, t_id: %d, b_index, %d \n", burstTime, t_id, b_index);
        // Send burst duration to queue
        pushBurst(rq, t_id, b_index, burstTime);
        pthread_cond_signal(&t_cond_wait);
        pthread_cond_wait(&(t_args[t_id].t_cond), &t_mutex); // wait cond mutex
        // Get Sleep Duration
        int sleepTime = getInterarrivalLength(NULL);
        usleep(sleepTime); // in ms *1000
        b_index++;
    }
    pushBurst(rq, t_id, Bcount, -1);
    pthread_cond_signal(&t_cond_wait);
    pthread_exit(0);
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

        ret = pthread_create(&t_args[i].tid,
                             &t_args[i].t_attr, do_task, (void *)&(t_args[i]));

        if (ret != 0)
        {
            printf("ERROR: thread create failed \n");
            exit(1);
        }
        printf("thread %i with tid %u created\n", i,
               (unsigned int)t_args[i].tid);
    }

    // serve thread logic
    int count = N;
    struct timeval startTime;
    struct timeval exeTime;
    while (count > 0)
    {
        // select node based on the ALG
        struct burst *node = getBurst(rq, alg);

        // handle the threads
        if (node == NULL)
        {
            pthread_mutex_lock(&serverMutex);
            pthread_cond_wait(&t_cond_wait, &serverMutex);
            pthread_mutex_unlock(&serverMutex);
            node = getBurst(rq, alg);
        }

        if (node->length <= 0)
        {
            count--;
            free(node);
            continue;
        }

        printf("(server) burst time: %d, t_id: %d, b_index, %d \n", node->length, node->thread_id, node->burst_id);
        // collect statistics
        usleep(node->length); // sleep till burst time *1000
        pthread_cond_signal(&(t_args[node->thread_id].t_cond));
        free(node);
    }

    printf("main: waiting all threads to terminate\n");
    for (int i = 0; i < N; i++)
    {
        printf("i %d\n", i);
        ret = pthread_join(t_args[i].tid, NULL);
        //ret = pthread_join(tids[i], NULL);
        if (ret != 0)
        {
            printf("thread join failed \n");
            exit(0);
        }
    }

    printf("main: all threads terminated\n");
    deleteReadyqueue(rq->head);
    free(rq);

    return 0;
}