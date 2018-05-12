#ifndef PAGE_H
#define PAGE_H

#include "lib/kernel/hash.h"
#include "devices/block.h"
#include "lib/stdbool.h"
#include "filesys/off_t.h"
#include "filesys/file.h"

typedef struct hash SupPageTable;

enum STATUS
{
  INSTALLED,
  SWAPPED,
  FSYS,
  ALLZERO
};

struct sptEntry
{
  void *page;
  void *frame;
  enum STATUS status;
  
  struct hash_elem hash_elem;
  block_sector_t block_idx;
  bool dirty;
  struct file *file;
  off_t ofs;
  uint32_t read_bytes;
  uint32_t zero_bytes;
  bool writable;
};

SupPageTable* spt_create (void);
bool spt_add_installed (SupPageTable*, void*, void*);
bool spt_add_filesys (SupPageTable*, void*,
			struct file*, off_t, uint32_t, uint32_t, bool);
bool spt_add_allzero (SupPageTable*, void*, void*);
bool spt_delete_entry (SupPageTable*, void*);
struct sptEntry* spt_get_entry (SupPageTable*, void*);
bool spt_set_swapped (SupPageTable*, void*, block_sector_t);
//bool spt_set_frame ();
//bool spt_set_status ();
//bool spt_set_block_idx ();

#endif
