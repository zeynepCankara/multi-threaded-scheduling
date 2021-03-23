
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include "readyqueue.h"

void fcfsTest(struct readyqueue *rq);
void sjfTest(struct readyqueue *rq);
void prioTest(struct readyqueue *rq);

#endif