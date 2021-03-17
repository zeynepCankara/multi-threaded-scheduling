
#ifndef READYQUEUE_H
#define READYQUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
    int val;
    struct node *next;
} node_t;

typedef struct readyqueue
{
    struct node *head;
    struct node *tail;
} readyqueue;

void print_list(node_t *head);
int pop(node_t **head);
int remove_last(node_t *head);
int remove_by_index(node_t **head, int n);
int remove_by_value(node_t **head, int val);
void delete_list(node_t *head);
void push(node_t **head, int val);

#endif