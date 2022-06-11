#include "include/sys_handler.h"



handler handler_func[SYS_N]={ \
    getpid_handler, uart_read_handler, uart_write_handler, \
    exec_handler, fork_handler, exit_handler, \
    mbox_call_handler, _kill_handler,\    
    signal_regis_handler, kill_handler, mmap_handler, \
    open_handler, close_handler, write_handler, read_handler, \
    mkdir_handler, mount_handler, chdir_handler, \
    empty_handler, empty_handler, \
    sync_handler, sigreturn_handler \
};



void sys_handler(uint64_t tf_base){ // x0
    save_sys_regs();

    uint32_t esr;
    asm volatile("mrs %0, esr_el1":"=r"(esr));
    if(esr>>26!=0b010101) {
        show_sys_regs();
        uart_hex(esr);
        while(1);
    }

    //uart_hex(tf_base);

    // Read Trap frame
    uint64_t k_sp;
    asm volatile("mov %0, sp":"=r"(k_sp));
    uint64_t offset = k_sp - tf_base;
    uint64_t tf[9];
    read_trap_frame(tf_base, tf);
    int sys_n = (int)tf[8];

    enable_int(); // kernel preemption
    handler_func[sys_n](tf); // RE-ENTRANT FREE
    disable_int();

    // Write Trap frame
    asm volatile("mov %0, sp":"=r"(k_sp));
    tf_base = k_sp - offset;
    write_trap_frame(tf_base, tf);
    
    load_sys_regs();
}

void getpid_handler(uint64_t* tf){
    thread* current = current_thread();
    tf[0] = current==NULL?-1:current->tpid;
}

void uart_read_handler(uint64_t* tf){
    char *buf = (char*) tf[0];
    size_t size = (size_t) tf[1];
    //while ((r_size=uart_async_read(buf, size))==0);
    int r_size = uart_sync_read(buf, size);
    
    tf[0] = r_size;
}

void uart_write_handler(uint64_t* tf){
    char *buf = (char*) tf[0];
    size_t size = (size_t) tf[1];
    //size_t w_size = uart_async_write(buf, size);
    size_t w_size = uart_sync_write(buf, size);
    tf[0] = (uint64_t)w_size;
}

void exec_handler(uint64_t* tf){
    const char* name = tf[0];
    char** argv = tf[1];
    int res = load_new_image(name, argv);
    tf[0] = res;
    from_el1_to_el0();
    
}

void fork_handler(uint64_t* tf){    
    
    int parent_pid = current_thread()->tpid;
    uint64_t k_sp;
    asm volatile("mov %0, sp":"=r"(k_sp));
    int pid = fork_process(k_sp);
    tf[0] = current_thread()->tpid==parent_pid?pid:0;
}

void exit_handler(uint64_t* tf){
    thread_exit();
}

void mbox_call_handler(uint64_t* tf){
    unsigned char channel = (unsigned char)tf[0];
    uint32_t* mbox = (uint32_t*)tf[1];
    int res = call_mailbox(channel, mbox);
    tf[0] = res;
}

void _kill_handler(uint64_t* tf){
    int tpid = tf[0];
    kill_thread(tpid);
}

void signal_regis_handler(uint64_t* tf){
    int SIG = (int)tf[0];
    uint64_t handler = tf[1];
    set_sig_handler(SIG, handler);
}

void kill_handler(uint64_t* tf){
    int pid = (int)tf[0];
    int SIG = (int)tf[1];
    add_sig_to_proc(pid, SIG);
}

void sigreturn_handler(uint64_t* tf){
    resume_sig_handler();
}

void mmap_handler(uint64_t* tf){
    void* addr = (void*)tf[0];
    size_t len = (size_t)tf[1];
    int prot = (int)tf[2];
    int flags = (int)tf[3];
    int fd = (int)tf[4];
    int file_offset = (int)tf[5];
    void* va = mmap(addr, len, prot, flags, fd, file_offset);
    tf[0] = (uint64_t)va;
}

void empty_handler(uint64_t* tf){
    
}

void open_handler(uint64_t* tf){
    char* pathname = (char*)tf[0];
    int flags = (int)tf[1];
    file* target;
    int fd = vfs_open(pathname, flags, &target);
    tf[0] = (uint64_t)fd;
}

void close_handler(uint64_t* tf){
    int fd = (int)tf[0];
    file* file = get_file_by_fd(fd);
    if (file == NULL){
        tf[0] = -1;
        return;
    }
    clear_fd(fd);
    int ret = vfs_close(file);
    tf[0] = (uint64_t)ret;
}

void write_handler(uint64_t* tf){
    int fd = (int)tf[0];
    void *buf = (void*)tf[1];
    int count = (int)tf[2]; 
    file* file = get_file_by_fd(fd);
    if (file == NULL){
        tf[0] = -1;
        return;
    }
    int w_size = vfs_write(file, buf, count);
    tf[0] = (int)w_size;
}

void read_handler(uint64_t* tf){
    int fd = (int)tf[0];
    void *buf = (void*)tf[1];
    int count = (int)tf[2]; 
    file* file = get_file_by_fd(fd);
    if (file == NULL){
        tf[0] = -1;
        return;
    }
    int r_size = vfs_read(file, buf, count);
    tf[0] = (int)r_size;
}

void mkdir_handler(uint64_t* tf){
    const char *pathname = (char*)tf[0];
    int ret = vfs_mkdir(pathname);
    tf[0] = (int)ret;
}

void mount_handler(uint64_t* tf){
    const char *src = (char*)tf[0];
    const char *target = (char*)tf[1];
    const char *filesystem = (char*)tf[2];
    unsigned long flags = (uint32_t)tf[3];
    const void *data = (void*)tf[4];
    int ret = vfs_mount(target, filesystem);
    tf[0] = (int)ret;
}

int chdir_handler(uint64_t* tf){
    const char *path = (char*)tf[0];
    int ret = vfs_chdir(path);
    tf[0] = ret;
}

void sync_handler(uint64_t* tf){
    fat32_sync();
}

void read_trap_frame(uint64_t base, uint64_t *tf){
    //uart_puts("tf base = ");
    //uart_hex(base);
    //uart_puts("\r\n");
    asm volatile("mov x10, %0"::"r"(base));
    asm volatile("ldr %0, [x10, 8*8]":"=r"(tf[8]));
    asm volatile("ldr %0, [x10, 8*7]":"=r"(tf[7]));
    asm volatile("ldr %0, [x10, 8*6]":"=r"(tf[6]));
    asm volatile("ldr %0, [x10, 8*5]":"=r"(tf[5]));
    asm volatile("ldr %0, [x10, 8*4]":"=r"(tf[4]));
    asm volatile("ldr %0, [x10, 8*3]":"=r"(tf[3]));
    asm volatile("ldr %0, [x10, 8*2]":"=r"(tf[2]));
    asm volatile("ldr %0, [x10, 8*1]":"=r"(tf[1]));
    asm volatile("ldr %0, [x10, 8*0]":"=r"(tf[0]));

   
}

void write_trap_frame(uint64_t base, uint64_t *tf){

    asm volatile("mov x10, %0"::"r"(base));
    asm volatile("str %0, [x10, 8*8]"::"r"(tf[8]));
    asm volatile("str %0, [x10, 8*7]"::"r"(tf[7]));
    asm volatile("str %0, [x10, 8*6]"::"r"(tf[6]));
    asm volatile("str %0, [x10, 8*5]"::"r"(tf[5]));
    asm volatile("str %0, [x10, 8*4]"::"r"(tf[4]));
    asm volatile("str %0, [x10, 8*3]"::"r"(tf[3]));
    asm volatile("str %0, [x10, 8*2]"::"r"(tf[2]));
    asm volatile("str %0, [x10, 8*1]"::"r"(tf[1]));
    asm volatile("str %0, [x10, 8*0]"::"r"(tf[0]));

}

void reset_trap_frame(uint64_t* tf){
    for(int i=0; i<9; i++) 
        tf[i] = 0;
}


