#ifndef __THREAD__
#define __THREAD__

#include "vfs.h"
#include "stdlib.h"
#include "mm.h"
#include "timer.h"
#include "sys_handler.h"
#include "vm.h"
#include "sig.h"

#define STACK_SIZE 4*PAGE_SIZE
#define THREAD_BLOCK_SIZE 16*8
#define RESERVED_MEM_THREAD 0x1000
#define MAX_THREAD 32//RESERVED_MEM_THREAD/THREAD_BLOCK_SIZE
#define PID_TAKEN 1
#define PID_FREE 0
#define MAX_FD 16


typedef struct sys_reg sys_reg;
struct sys_reg
{
    uint64_t spsr;
    uint64_t elr;
    uint64_t sp_el0;
};


typedef void* (*thread_callback)(void*);

typedef struct thread thread;
typedef uint64_t* pagetable;
typedef void (*sig_callback)(void);

typedef struct vnode vnode;
typedef struct file file;

struct thread
{
    int tpid;
    uint32_t priority;
    pagetable u_PGD;
    pagetable k_PGD;
    uint64_t u_stack_pa;
    uint64_t k_stack_pa;
    uint64_t u_text_pa;
    thread* next;
    thread* prev;
    sys_reg sys_reg;
    sig_callback registered_handler[255];
    uint32_t proc_size;

    // vfs
    vnode* current_dir_node;
    file* fd_table[MAX_FD];
};



/* Top level API */

void thread_init();

thread* thread_create(thread_callback callback, char **argv, uint64_t k_sp, uint32_t priority);
void thread_exit();
void periodical_schedule();
void schedule();
void kill_thread(int tpid);

/* Util */
void switch_to(uint64_t prev, uint64_t next, uint64_t k_sp);

uint64_t get_current_tpid_offset();
int get_free_tpid();
void set_tpid_free(int tpid);
void tpid_offset_init();

thread* Thread(int tpid, uint32_t priority, uint64_t k_stack_pa, uint64_t u_stack_pa, uint64_t u_text_pa, pagetable k_PGD, pagetable u_PGD);
void thread_enqueue(thread* head, thread* p);
thread* thread_dequeue(thread* head);
thread* current_thread();
thread* get_thread(int tpid);

thread* thread_dequeue_by_id(thread* head, int tpid);
void thread_enqueue_run_queue(thread* t);

void idle();
void kill_zombies();

uint64_t cp_stack(uint64_t c_stack, uint64_t p_stack, uint64_t p_sp);


void save_sys_regs();
void load_sys_regs();
void show_sys_regs();




#endif