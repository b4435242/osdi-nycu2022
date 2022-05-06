#include "include/task.h"

task task_queues[MAX_THREAD]; 
int priority_stacks[MAX_THREAD][10];
int nested_arr[MAX_THREAD];

void add_task(callback_ptr callback, int priority){
    task* t = Task(callback, priority);
    
    disable_int();
    // enqueue based on priority
    task *tq = &task_queues[current_thread()->tpid];
    task_enqueue(tq, t);
    
    while (exec_task(tq));

    enable_int();
}

int exec_task(task* tq){
    task* t = task_dequeue(tq);
    
    int curr = current_thread()->tpid;
    int *nested = &nested_arr[curr];
    int *priority_stack = &priority_stacks[curr];

    if (t==NULL) return 0;
    if (*nested>0 && t->priority>=priority_stack[*nested-1]) return 0;
    
    priority_stack[*nested++] = t->priority;
    
    // do the task
    enable_int();
    t->callback(NULL);
    disable_int();

    *nested--;

    free(t);
    return 1;
}

task* Task(callback_ptr callback, int priority){
    task* t = malloc(sizeof(task));
    t->callback = callback;
    t->priority = priority;
    t->next = NULL;
    t->prev = NULL;
    return t;
}


/* head is NOT NULL */
void task_enqueue(task* head, task* p){
    task *cur = head->next;
    task *prev = head;
    while (cur!=NULL && p->priority>=cur->priority) 
    {
        prev = cur;
        cur = cur->next;
    }

    // prev <=> p <=> cur 
    prev->next = p;
    p->prev = prev;
    p->next = cur;
    if (cur!=NULL)
        cur->prev = p;
    
}

task* task_dequeue(task* head){
    task *first = head->next;
    if (first!=NULL){
        // prev <=> p <=> next 
        task* prev = first->prev;
        task* next = first->next;

        // prev <=> next 
        prev->next = next;
        next->prev = prev;
    }
    return first;
}

task* task_queue_top(task* head){
    task *first = head->next;
    return first;
}