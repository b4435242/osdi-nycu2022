#ifndef __RESET__
#define __RESET__

void reset(int tick);
void cancel_reset();
void set(long addr, unsigned int value);

#endif