#ifndef SWAP_H
#define SWAP_H

#include "devices/block.h"

void swap_init (void);
void swap_out (void *);
void swap_in (void *, void *);

#endif
