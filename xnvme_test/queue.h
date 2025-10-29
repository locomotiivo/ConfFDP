#ifndef QUEUE_H
#define QUEUE_H

/** custom implementation of basic queue
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct queue queue;
queue *queue_init();
queue *queue_push(queue *q, void *data);
queue *queue_pop(queue *q);
void *queue_peek(queue *q);
void queue_destroy(queue *q);
bool isEmpty(queue *q);

#endif // QUEUE_H