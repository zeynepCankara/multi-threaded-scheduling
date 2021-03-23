/**
 * Ready queue to select CPU bursts based on the selected algorithm.
 * Internally implements a linkedlist to execute readyqueue operations
 * @author Zeynep Cankara - 21703381
 * @version 1.0
 */

#include "readyqueue.h"

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t rqLock;

// initialises the ready queue
struct readyqueue *initReadyQueue()
{
    struct readyqueue *rq = malloc(sizeof(struct readyqueue));
    rq->head = NULL;
    rq->tail = NULL;
    if (pthread_mutex_init(&rqLock, NULL) != 0)
    {
        printf("ERROR: mutex lock can't initialise readyqueue!\n");
        return NULL;
    }
    return rq;
}

// initialises a burst node
struct burst *createBurst(int thread_id, int burst_id, int length)
{
    struct burst *node = malloc(sizeof(struct burst));
    node->thread_id = thread_id;
    node->burst_id = burst_id;
    node->length = length;
    node->next = NULL;
    return node;
}

/* Pushes a CPU burst to the readyqueue
** Uses mutex locker to block concurrent access to the critical section
*/
void pushBurst(struct readyqueue *rq, int thread_id, int burst_id, int length)
{
    pthread_mutex_lock(&rqLock);
    struct timeval time;
    struct burst *node = createBurst(thread_id, burst_id, length);
    if (rq->head != NULL) // readyqueue is not empty
    {
        rq->tail->next = node;
        rq->tail = rq->tail->next;
    }
    else
    {
        rq->head = node;
        rq->tail = rq->head;
    }
    gettimeofday(&time, NULL);
    rq->tail->time = time;
    pthread_mutex_unlock(&rqLock);
}

// >ALG>: First Come First Serve
struct burst *fcfs(struct readyqueue *rq)
{
    pthread_mutex_lock(&rqLock);
    if (rq->head == NULL)
    {
        pthread_mutex_unlock(&rqLock);
        return NULL;
    }
    struct burst *headBurst = rq->head;
    rq->head = headBurst->next;
    if (headBurst == rq->tail)
    {
        rq->tail = NULL;
    }
    pthread_mutex_unlock(&rqLock);
    headBurst->next = NULL;
    return headBurst;
}

// <ALG>: Short Job First
struct burst *sjf(struct readyqueue *rq)
{
    pthread_mutex_lock(&rqLock);
    if (rq->head == NULL)
    {
        pthread_mutex_unlock(&rqLock);
        return NULL;
    }
    if (rq->head == rq->tail)
    {
        struct burst *node = rq->head;
        rq->head = NULL;
        rq->tail = NULL;
        pthread_mutex_unlock(&rqLock);
        return node;
    }
    struct burst *currNode = rq->head->next;
    struct burst *prevNode = rq->head;
    struct burst *prevMinLengthNode = NULL;
    struct burst *minLengthNode = rq->head;
    int minLength = minLengthNode->length;

    while (currNode != NULL)
    {
        if (currNode->length < minLength)
        {
            minLengthNode = currNode;
            prevMinLengthNode = prevNode;
            minLength = minLengthNode->length;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }

    if (minLengthNode == rq->head)
    {
        rq->head = rq->head->next;
    }

    else if (minLengthNode == rq->tail)
    {
        rq->tail = prevMinLengthNode;
        rq->tail->next = NULL;
    }

    else
    {
        prevMinLengthNode->next = minLengthNode->next;
    }

    pthread_mutex_unlock(&rqLock);
    minLengthNode->next = NULL; // avoid access to NULL next
    return minLengthNode;
}

// prints content of the readyqueue
void printReadyqueue(burst *head)
{
    burst *node = head;

    while (node != NULL)
    {
        printf("burst id: %d\n", node->burst_id);
        node = node->next;
    }
}

// deletes the readyqueue by deallocation
void deleteReadyqueue(struct burst *head)
{
    struct burst *current = head,
                 *next = head;

    while (current)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

struct burst *getBurst(struct readyqueue *rq, char *alg)
{
    if (strcmp(alg, "FCFS") == 0)
    {
        return fcfs(rq);
    }
    else if (strcmp(alg, "SJF") == 0)
    {
        return sjf(rq);
    }
    else
    {
        printf("ERROR: algorithm not available \n");
        printf("Options <ALG>: FCFS, SJF \n");
        return NULL;
    }
}