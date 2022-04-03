#include "include/task.h"

Task task_queue[100];
int task_queue_size = 0;

int priority_stack[100];
int nested_size = 0;

void add_task(callback_ptr callback, int priority){
    Task task = {
        .callback = callback,
        .priority = priority
    };
    
    disable_int();
    // enqueue based on priority
    int idx = task_queue_size;
    for(int i=0; i<task_queue_size; i++){
        if (task.priority<task_queue[i].priority){
            idx = i;
            break;
        }
    }

    task_queue_size++;
    for(int i=idx+1; i<task_queue_size; i++){
        task_queue[i] = task_queue[i-1];
    }
    task_queue[idx] = task;
    // enqueue complete

    while (exec_task());

    enable_int();
}

int exec_task(){
    if (task_queue_size==0) return 0;
    Task task = task_queue[0];

    if (nested_size>0 && task.priority>=priority_stack[nested_size-1]) return 0;
    
    
    priority_stack[nested_size++] = task.priority;

    
    // do the task
    enable_int();
    task.callback(NULL);
    disable_int();

    nested_size--;

    dequeue_task();
    return 1;
}


void dequeue_task(){
    // clean up finished task
    // always the head of queue
    for(int i=1; i<task_queue_size; i++){
        task_queue[i-1] = task_queue[i];
    }
    task_queue_size--;
}