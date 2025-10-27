#include "queue.h"

/** custom implementation of basic queue
 */
#include <string.h>

typedef struct node {
    void *data;
    struct node *next;
} node;


typedef struct queue {
    size_t size;
    node *head;
    node *tail;
} queue;

node *create_node(void *data, size_t size) {
    node *new = (node *)malloc(sizeof(node));
    if (!new) return NULL;
    new->data = malloc(size);
    if (!new->data) {
        free(new);
        return NULL;
    }
    memcpy(new->data, data, size);
    new->next = NULL;
    return new;
}

queue *queue_init() {
    queue *q = (queue *)malloc(sizeof(queue));
    if (!q) return NULL;
    q->size = 0;
    q->head = q->tail = NULL;
    return q;
}

queue *push(queue *q, void *data) {
    if (!q) return NULL;
    node *new = create_node(data, sizeof(data));
    if (!new) return NULL;
    if (isEmpty(q)) {
        q->head = q->tail = new;
    } else {
        q->tail->next = q->tail = new;
    }
    q->size++;
    return q;
}

queue *pop(queue *q) {
    if (!q || isEmpty(q)) {
        return NULL;
    }
    node *tmp = q->head;
    if (q->size == 1) {
        q->head = q->tail = NULL;
    } else {
        q->head = q->head->next;
    }

    q->size--;
    free(tmp->data);
    free(tmp);
    return q;
}

void* queue_peek(queue *q) {
    if (!q || isEmpty(q)) return NULL;
    return q->head;
}

void queue_destroy(queue *q) {
    if (!q) return;
    while (!isEmpty(q)) {
        pop(q);
    }
    free(q);
}

bool isEmpty(queue *q) {
    return (!q || q->size == 0); 
}
