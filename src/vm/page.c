#include "page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "lib/stdbool.h"
#include "lib/kernel/hash.h"

static unsigned
spt_hash (const struct hash_elem *element, void *aux UNUSED)
{
  struct sptEntry *spte = hash_entry (element, struct sptEntry, hash_elem);
  return hash_bytes (&spte->page, sizeof (void *));
}

static bool
spt_less (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct sptEntry *spte_a = hash_entry (a, struct sptEntry, hash_elem);
  struct sptEntry *spte_b = hash_entry (b, struct sptEntry, hash_elem);
  
  return spte_a->page < spte_b->page;
}

SupPageTable*
spt_create ()
{
  SupPageTable *spt = malloc (sizeof (SupPageTable));
  if (spt == NULL)
    PANIC ("not enough memory for spt");
  hash_init (spt, spt_hash, spt_less, NULL);
  return spt;
}

bool
spt_add_installed (SupPageTable *spt, void *page, void *frame)
{
  struct sptEntry *new;
  new = malloc (sizeof (struct sptEntry));
  new->page = page;
  new->frame = frame;
  new->status = INSTALLED;
  new->dirty = false;
  new->writable = true;

  return hash_insert (spt, &new->hash_elem) == NULL;
}

bool
spt_add_filesys (SupPageTable *spt, void *page, struct file *file,
		 off_t offset, uint32_t read_bytes, uint32_t zero_bytes,
		 bool writable)
{
  struct sptEntry *new;
  new = malloc (sizeof (struct sptEntry));
  new->page = page;
  new->file = file;
  new->ofs = offset;
  new->read_bytes = read_bytes;
  new->zero_bytes = zero_bytes;
  new->writable = writable;
  new->status = FSYS;
  new->dirty = false;

  return hash_insert (spt, &new->hash_elem) == NULL;
}

bool
spt_add_allzero (SupPageTable *spt, void *page, void *frame)
{
  struct sptEntry *new;
  new = malloc (sizeof (struct sptEntry));
  new->page = page;
  new->frame = frame;
  new->status = ALLZERO;
  new->dirty = false;
  new->writable = true;

  return hash_insert (spt, &new->hash_elem) == NULL;
}

bool
spt_delete_entry (SupPageTable *spt, void *page)
{
  struct sptEntry del;
  struct hash_elem *del_elem;
  
  del.page = page;
  del_elem = hash_find (spt, &del.hash_elem);
  return hash_delete (spt, del_elem) != NULL;
}

struct sptEntry*
spt_get_entry (SupPageTable *spt, void *page)
{
  struct sptEntry tmp;
  struct hash_elem *tmp_elem;

  tmp.page = page;
  tmp_elem = hash_find (spt, &tmp.hash_elem);
  if (tmp_elem == NULL) return NULL;
  return hash_entry (tmp_elem, struct sptEntry, hash_elem);
}

bool
spt_set_swapped (SupPageTable *spt, void *page, block_sector_t block_idx)
{
  struct sptEntry *target;
  target = spt_get_entry (spt, page);
  
  if (target == NULL) return false;
  target->frame = NULL;
  target->status = SWAPPED;
  target->block_idx = block_idx;
  return true;
}


