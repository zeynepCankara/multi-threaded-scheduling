
#ifndef READYQUEUE_H
#define READYQUEUE_H

#include <sys/time.h> // gettimeoftheday()
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct burst
{
    struct burst *next;
    int thread_id;
    int burst_id;
    int length;
    struct timeval time;
} burst;

typedef struct readyqueue
{
    struct burst *head;
    struct burst *tail;
} readyqueue;

struct readyqueue *initReadyQueue();
struct burst *createBurst(int thread_id, int burst_id, int length);
void pushBurst(struct readyqueue *rq, int thread_id, int burst_id, int length);
struct burst *getBurst(struct readyqueue *rq, char *alg);
struct burst *sjf(struct readyqueue *rq);
struct burst *fcfs(struct readyqueue *rq);
struct burst *prio(struct readyqueue *rq);
struct burst *vruntime(struct readyqueue *rq);

// helpers
void printReadyqueue(burst *head);
void deleteReadyqueue(struct burst *head);

#endif