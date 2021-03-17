/**
 * Ready queue to select CPU bursts based on the selected algorithm.
 * Internally implements a linkedlist to execute readyqueue operations
 * @author Zeynep Cankara - 21703381
 * @version 1.0
 */

#include "readyqueue.h"

void print_list(node_t *head)
{
    node_t *current = head;

    while (current != NULL)
    {
        printf("%d\n", current->val);
        current = current->next;
    }
}

int pop(node_t **head)
{
    int retval = -1;
    node_t *next_node = NULL;

    if (*head == NULL)
    {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}

int remove_last(node_t *head)
{
    int retval = 0;
    /* if there is only one item in the list, remove it */
    if (head->next == NULL)
    {
        retval = head->val;
        free(head);
        return retval;
    }

    /* get to the second to last node in the list */
    node_t *current = head;
    while (current->next->next != NULL)
    {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->val;
    free(current->next);
    current->next = NULL;
    return retval;
}

int remove_by_index(node_t **head, int n)
{
    int i = 0;
    int retval = -1;
    node_t *current = *head;
    node_t *temp_node = NULL;

    if (n == 0)
    {
        return pop(head);
    }

    for (i = 0; i < n - 1; i++)
    {
        if (current->next == NULL)
        {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->val;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}

int remove_by_value(node_t **head, int val)
{
    node_t *previous, *current;

    if (*head == NULL)
    {
        return -1;
    }

    if ((*head)->val == val)
    {
        return pop(head);
    }

    previous = current = (*head)->next;
    while (current)
    {
        if (current->val == val)
        {
            previous->next = current->next;
            free(current);
            return val;
        }

        previous = current;
        current = current->next;
    }
    return -1;
}

void delete_list(node_t *head)
{
    node_t *current = head,
           *next = head;

    while (current)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

void push(node_t **head, int val)
{
    node_t *new_node;
    new_node = (node_t *)malloc(sizeof(node_t));

    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}
