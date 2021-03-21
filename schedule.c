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
#include "readyqueue.h"

#define MAXTHREADS 20  /* max number of threads */
#define MAXFILENAME 50 /* max length of a filename */

/*
*  thread function will take a pointer to this structure
*/
struct arg
{
    int n;       /* min value */
    int m;       /* max value */
    int t_index; /* the index of the created thread */
};

/* this is function to be executed by the threads */
static void *do_task(void *arg_ptr)
{
    int i;
    FILE *fp;
    char filename[MAXFILENAME];

    printf("thread %d started\n", ((struct arg *)arg_ptr)->t_index);

    sprintf(filename, "output_of_thread%d.txt",
            ((struct arg *)arg_ptr)->t_index);

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        perror("do_task:");
        exit(1);
    }

    for (i = ((struct arg *)arg_ptr)->n;
         i <= ((struct arg *)arg_ptr)->m; ++i)
    {
        fprintf(fp, "integer = %d\n", i);
    }

    fclose(fp);

    pthread_exit(NULL);
}

void testReadyQueue(struct readyqueue *rq)
{
    pushBurst(rq, 1, 2, 3);
    pushBurst(rq, 2, 3, 4);
    pushBurst(rq, 3, 22, 5);
    printReadyqueue(rq->head);
    struct burst *b = fcfs(rq);
    pushBurst(rq, 3, 4, 5);
    pushBurst(rq, 4, 5, 6);
    printReadyqueue(rq->head);
    deleteReadyqueue(rq->head);
}

int main(int argc, char *argv[])
{
    int isFromFile = 0;
    int N;
    int Bcount;
    int minB;
    int avgB;
    int minA;
    int avgA;
    char *alg;
    char *inprefix;
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
        printf("%d %d %d %d %d %d %s\n", N, Bcount, minB, avgB, minA, avgA, alg);
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
        printf("%d %s %s\n", N, alg, inprefix);
    }
    else
    {
        printf("Error: Number of arguments does not match with any mode.\n");
        printf("read from terminal mode:$ schedule <N> <Bcount> <minB> <avgB> <minA> <avgA> <ALG>\n");
        printf("read from file mode:$ schedule <N> <ALG> -f <inprefix>\n");
        return -1;
    }

    // test the readyqueue
    struct readyqueue *rq = initReadyQueue();
    testReadyQueue(rq);

    return 0;
}