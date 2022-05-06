#ifndef __SIG__
#define __SIG__

#include "thread.h"
#include "sys.h"

#define MAX_SIG 255
#define SIGKILL 9

typedef struct sig sig;
struct sig
{
    int SIG;
    sig* prev;
    sig* next;
};

typedef struct context context;


struct context
{
    struct sys_reg *sys_regs;
    uint64_t gen_regs[13];
};

typedef void (*sig_callback)(void);


void sig_handler();
void set_sig_handler(int SIG, uint64_t handler);
void cp_sig_handler(int parent, int child);

void add_sig_to_proc(int pid, int SIG);
void resume_sig_handler();

void sig_enqueue(sig* head, sig* p);
sig* sig_dequeue(sig* head);
sig* sig_get_top(sig* head);

inline void save_context()__attribute__ ((always_inline)) ;
inline void load_context()__attribute__ ((always_inline)) ;
 

#endif