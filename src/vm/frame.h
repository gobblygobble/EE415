#ifndef FRAME_H
#define FRAME_H

#include "lib/kernel/hash.h"
#include "lib/kernel/list.h"
#include "threads/palloc.h"
#include "threads/thread.h"

typedef struct hash frameTable;

struct frameTableEntry {
  void *frame;
  void *page;
  uint32_t *pagedir;

  struct hash_elem hash_elem;
  struct list_elem list_elem;
};

void ft_init (void);
void* frame_alloc (enum palloc_flags, void*);
void frame_free (void*);

#endif
