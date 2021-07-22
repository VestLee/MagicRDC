#ifndef _RELOCATE_H
#define _RELOCATE_H
#include <stdio.h>


#if (__arm__) /*armv7*/
void relocateInstruction(uint32_t target_addr, void *orig_instructions, int length, void *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count);

#endif
#endif
