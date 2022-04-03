#ifndef __TASK__
#define __TASK__

#include "stdlib.h"
#include "irq.h"

typedef struct _Task {
    callback_ptr callback;
    int priority;
} Task;


void add_task(callback_ptr callback, int priority);
void dequeue_task();
int exec_task();


#endif