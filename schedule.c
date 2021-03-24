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

#define MAX_THREADS 10   // maximum number of threads
#define MAX_BCOUNT 10000 // maximum number of threads
#define LINE_LEN 10000   // length of a line

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
int isFromFile;

// for thread management
pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

int t_vruntime[MAX_THREADS] = {0}; // virtual runtime of each thread

// argument list for the thread
typedef struct argvThread
{
    int t_index;   // thread index
    pthread_t tid; // thread id
    pthread_attr_t t_attr;
    pthread_cond_t t_cond; // condition variable
} argvThread;

struct argvThread t_args[MAX_THREADS]; // pass the thread arguments
pthread_cond_t t_cond_wait = PTHREAD_COND_INITIALIZER;

// for keeping statistics
int threadTotalWaitingTime[MAX_THREADS];
int burstTotalWaitingTime[MAX_BCOUNT];

// keep track of the reading from the file and thread
FILE *readFromThread[MAX_THREADS];
char *readFromLine[MAX_THREADS] = {NULL};
size_t readFromLen[MAX_THREADS] = {0};

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

int getBurstLength()
{
    int length = 0;

    length = getRandExpTime(avgB, minB);

    return length;
}

int getInterarrivalLength()
{
    int length = 0;

    length = getRandExpTime(avgA, minA);

    return length;
}

void getTimeFromFile(int t_id, char *timeBuffer[])
{
    // returns both burst time and sleep time
    //int length = 0;
    FILE *fp = readFromThread[t_id - 1];
    ssize_t isLine;
    if ((isLine = getline(&readFromLine[t_id - 1], &readFromLen[t_id - 1], fp)) != -1)
    {
        char buf[LINE_LEN];
        sprintf(buf, "%zu", isLine);
        int i = 0;
        char *p = strtok(buf, " ");
        //char *timeBuffer[2]; // interarrival burst
        while (p != NULL)
        {
            timeBuffer[i++] = p;
            p = strtok(NULL, "/");
        }

        for (i = 0; i < 2; ++i)
            printf("%s\n", timeBuffer[i]);
    }
}

static void *do_task(void *arg_ptr)
{
    int t_id = ((struct argvThread *)arg_ptr)->t_index;
    pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;
    printf("thread %d started\n", t_id);

    // perform task...
    int b_index = 1;
    while (b_index <= Bcount)
    {
        // Get Burst Duration
        int burstTime;
        // Get Sleep Duration
        int sleepTime;
        if (isFromFile == 0)
        {
            burstTime = getBurstLength();
            sleepTime = getInterarrivalLength();
        }
        else
        {
            char *timeBuffer[2];
            getTimeFromFile(t_id, timeBuffer);
            burstTime = atoi(timeBuffer[0]);
            sleepTime = atoi(timeBuffer[1]);
            printf(" (do_task)-burstTime: %d\n", burstTime);
            printf(" (do_task)-sleepTime: %d\n", sleepTime);
        }
        printf(" (do_task)-burstTime: %d, t_id: %d, b_index, %d \n", burstTime, t_id, b_index);
        // Send burst duration to queue
        pushBurst(rq, t_id, b_index, burstTime);
        pthread_cond_signal(&t_cond_wait);
        pthread_cond_wait(&(t_args[t_id].t_cond), &t_mutex); // wait cond mutex
        usleep(sleepTime * 1000);                            // in ms
        b_index++;
    }
    pushBurst(rq, t_id, Bcount, -1);
    pthread_cond_signal(&t_cond_wait);
    pthread_exit(0);
}

// Main Program
int main(int argc, char *argv[])
{
    isFromFile = 0;
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
    if (isFromFile)
    {
        // read the contents of the input files
        FILE *fp;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        for (int i = 1; i <= N; i++)
        {
            char threadInprefix[100];
            strcpy(threadInprefix, inprefix);
            strcat(threadInprefix, "-");
            char N_str[100];
            sprintf(N_str, "%d", N);
            strcat(threadInprefix, N_str);
            strcat(threadInprefix, ".txt");
            fp = fopen(threadInprefix, "r");
            if (fp == NULL)
                exit(EXIT_FAILURE);
            printf("attaching file to thread: %d\n", i);
            readFromThread[i - 1] = fp;
        }

        fp = fopen("./afile-1.txt", "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);
        Bcount = 0;
        while ((read = getline(&line, &len, fp)) != -1)
        {
            Bcount += 2;
            printf("Retrieved line of length %zu:\n", read);
            printf("%s", line);
        }

        fclose(fp);
        if (line)
            free(line);
        //exit(EXIT_SUCCESS);
    }

    // initialise statistics variables
    for (int i = 0; i < N; i++)
    {
        threadTotalWaitingTime[i] = 0;
    }
    for (int i = 0; i < Bcount; i++)
    {
        burstTotalWaitingTime[i] = 0;
    }

    // create the readyqueue
    rq = initReadyQueue();

    int ret;

    for (int i = 0; i < N; ++i)
    {
        t_args[i].t_index = i + 1;
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

    // server thread logic
    int count = N;
    // time the burst start execution
    struct timeval timeStart;
    // time the burst exec finished
    struct timeval timeFinish;
    int burstProcessed = 0;
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

        if (node->length <= 0) // node processed
        {
            // update virtual runtime
            if (strcmp(alg, "VRUNTIME") == 0)
            {
                int t_id = node->thread_id;
                int burst_time = node->length;
                t_vruntime[t_id] += (burst_time * (0.7 + 0.3 * t_id));
            }
            // update the state
            count--;
            free(node);
            continue;
        }
        // node info
        printf("(server) burst time: %d, t_id: %d, b_index, %d \n", node->length, node->thread_id, node->burst_id);

        // collect statistics
        if (burstProcessed != 1)
        {
            gettimeofday(&timeStart, NULL);
            timeFinish = timeStart;
            burstProcessed = 1;
        }
        else
        {
            gettimeofday(&timeFinish, NULL);
        }
        int threadWaitingTime = ((timeFinish.tv_sec - node->time.tv_sec) * 1000000 + (timeFinish.tv_usec - node->time.tv_usec)) / 1000;
        threadTotalWaitingTime[node->thread_id - 1] += threadWaitingTime;
        burstTotalWaitingTime[node->burst_id - 1] += threadWaitingTime;

        // collect statistics
        usleep(node->length * 1000); // sleep till burst time
        pthread_cond_signal(&(t_args[node->thread_id].t_cond));
        free(node);
    }

    printf("Waiting threads to terminate...\n");
    for (int i = 0; i < N; i++)
    {
        ret = pthread_join(t_args[i].tid, NULL);
        //  report the statistics
        printf("\tTotal waiting time in thread %d: %d ms.\n", (i + 1), threadTotalWaitingTime[i]);
        if (ret != 0)
        {
            printf("thread join failed \n");
            exit(0);
        }
    }
    printf("main: all threads terminated\n");
    for (int i = 0; i < Bcount; i++)
    {
        printf("\tTotal waiting time of burst %d: %d ms.\n", (i + 1), burstTotalWaitingTime[i]);
    }

    // deallocate the ready queue
    deleteReadyqueue(rq->head);
    free(rq);

    return 0;
}