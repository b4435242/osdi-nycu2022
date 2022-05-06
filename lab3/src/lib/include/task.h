#ifndef __TASK__
#define __TASK__

#include "stdlib.h"
#include "irq.h"
#include "thread.h"

typedef struct task task;
struct task {
    callback_ptr callback;
    int priority;
    task* next;
    task* prev;
};


void add_task(callback_ptr callback, int priority);
int exec_task(task* tq);
void task_enqueue(task* head, task* p);
task* task_dequeue(task* head);
task* task_get_top(task* head);
task* Task(callback_ptr callback, int priority);

#endif