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

pthread_mutex_t rq_lock;

// initialises the ready queue
struct readyqueue *initReadyQueue()
{
    struct readyqueue *rq = malloc(sizeof(struct readyqueue));
    rq->head = NULL;
    rq->tail = NULL;
    if (pthread_mutex_init(&rq_lock, NULL) != 0)
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
    pthread_mutex_lock(&rq_lock);
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
    pthread_mutex_unlock(&rq_lock);
}

// pops a cpu burst out of the readyqueue
struct burst *popBurst(struct burst *head)
{
    struct burst *node = createBurst(-1, -1, -1); // create a node with default values

    if (head->next == NULL)
    {
        node->thread_id = head->thread_id;
        node->burst_id = head->burst_id;
        node->length = head->length;
        free(head);
        return node;
    }

    /* get to the second to last node in the list */
    struct burst *current = head;
    while (current->next->next != NULL)
    {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    node->thread_id = current->next->thread_id;
    node->burst_id = current->next->burst_id;
    node->length = current->next->length;
    free(current->next);
    current->next = NULL;
    return node;
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
