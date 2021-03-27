/**
 * Test functions for the scheduling algorithms
 * @author Zeynep Cankara - 21703381
 * @version 1.0
 */

#include "test.h"

void fcfsTest(struct readyqueue *rq)
{
    pushBurst(rq, 1, 1, 3);
    pushBurst(rq, 2, 2, 4);
    pushBurst(rq, 3, 3, 5);
    printReadyqueue(rq->head);
    printf("(1, 2, 3) printed \n");
    fcfs(rq);
    pushBurst(rq, 3, 4, 5);
    pushBurst(rq, 4, 5, 6);
    printReadyqueue(rq->head);
    printf("(2, 3, 4, 5) printed \n");
    fcfs(rq);
    fcfs(rq);
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
    sjf(rq);
    pushBurst(rq, 1, 3, 3);
    pushBurst(rq, 1, 5, 5);
    printReadyqueue(rq->head);
    printf("(8, 9, 3, 5) printed \n");
    sjf(rq);
    sjf(rq);
    pushBurst(rq, 1, 2, 2);
    pushBurst(rq, 1, 1, 1);
    printReadyqueue(rq->head);
    printf("(8, 9, 2, 1) printed \n");
    deleteReadyqueue(rq->head);
}

void prioTest(struct readyqueue *rq)
{
    pushBurst(rq, 8, 8, 8);
    pushBurst(rq, 7, 7, 7);
    pushBurst(rq, 9, 9, 9);
    printReadyqueue(rq->head);
    printf("(8, 7, 9) printed \n");
    prio(rq);
    pushBurst(rq, 3, 3, 3);
    pushBurst(rq, 5, 5, 5);
    printReadyqueue(rq->head);
    printf("(8, 9, 3, 5) printed \n");
    prio(rq);
    prio(rq);
    pushBurst(rq, 2, 2, 2);
    pushBurst(rq, 1, 1, 1);
    printReadyqueue(rq->head);
    printf("(8, 9, 2, 1) printed \n");
    deleteReadyqueue(rq->head);
}