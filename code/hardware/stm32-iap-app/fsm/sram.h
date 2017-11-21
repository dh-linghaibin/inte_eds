#ifndef __SRAM_H
#define __SRAM_H		

#define debug_pt(...) //printf(__VA_ARGS__)

void *SramMalloc(unsigned int size);
void SramFree(void *pv);
void TraceHeap(void);
#endif
