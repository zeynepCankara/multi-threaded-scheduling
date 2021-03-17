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
    node_t *test_list = (node_t *)malloc(sizeof(node_t));

    test_list->val = 1;
    test_list->next = (node_t *)malloc(sizeof(node_t));
    test_list->next->val = 2;
    test_list->next->next = (node_t *)malloc(sizeof(node_t));
    test_list->next->next->val = 3;
    test_list->next->next->next = (node_t *)malloc(sizeof(node_t));
    test_list->next->next->next->val = 4;
    test_list->next->next->next->next = NULL;

    remove_by_value(&test_list, 3);

    print_list(test_list);
    delete_list(test_list);

    return 0;
}