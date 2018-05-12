#include "frame.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/list.h"
#include "lib/stdbool.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
static frameTable ft;
static struct list all_frames;
static struct lock frame_alloc_lock;
static struct lock frame_free_lock;

static unsigned
ft_hash (const struct hash_elem *element, void *aux UNUSED)
{
  struct frameTableEntry *fte = hash_entry (element, struct frameTableEntry, hash_elem);
  return hash_bytes (&fte->frame, sizeof (void*));
}

static bool
ft_less (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct frameTableEntry *fte_a = hash_entry (a, struct frameTableEntry, hash_elem);
  struct frameTableEntry *fte_b = hash_entry (b, struct frameTableEntry, hash_elem);
  
  return fte_a->frame < fte_b->frame;
}

void
ft_init ()
{
  hash_init (&ft, ft_hash, ft_less, NULL);
  list_init (&all_frames);
  lock_init (&frame_alloc_lock);
  lock_init (&frame_free_lock);
}

static bool
ft_add_entry (void *frame, void *page, uint32_t *pagedir)
{
  struct frameTableEntry *fte;
  fte = malloc (sizeof (struct frameTableEntry));
  if (fte == NULL) return false;
  fte->frame = frame;
  fte->pagedir = pagedir;
  fte->page = page;

  if (hash_insert (&ft, &fte->hash_elem) == NULL) {
    list_push_back (&all_frames, &fte->list_elem);
    return true;
  }
  return false;
}

static struct frameTableEntry*
ft_get_entry (void *frame)
{
  struct frameTableEntry fte;
  struct hash_elem *fte_elem;

  fte.frame = frame;
  fte_elem = hash_find (&ft, &fte.hash_elem);

  return hash_entry (fte_elem, struct frameTableEntry, hash_elem);
}

static bool
ft_delete_entry (void *frame)
{
  struct frameTableEntry *fte = ft_get_entry (frame);
  if (hash_delete (&ft, &fte->hash_elem) == NULL
	|| list_remove (&fte->list_elem) == NULL)
    return false;
  return true;
}

static void*
select_victim (void)
{
  ASSERT (!list_empty (&all_frames));

  struct list_elem *victim_elem = list_pop_front (&all_frames);
  list_push_back (&all_frames, victim_elem);
  
  struct frameTableEntry *victim = list_entry (victim_elem, struct frameTableEntry, list_elem);
  return victim;
/*
  int len = (int)list_size (&all_frames);
  // clock algorithm
  while (len--) {
    struct list_elem *victim = list_pop_front (&all_frames);
    list_push_back (&all_frames, victim);
    //NOT_YET
    struct frameTableEntry *victim_fte = list_entry (victim, struct frameTableEntry, list_elem);
  
    if (pagedir_is_dirty (victim_fte->pagedir, victim_fte->page))
      return victim_fte->frame;
  }
  return NULL;
*/
}

void*
frame_alloc (enum palloc_flags flags, void *page)
{
  lock_acquire (&frame_alloc_lock);
  void *frame = palloc_get_page (flags);

  if (frame == NULL) {
    struct frameTableEntry *victim = select_victim ();
    swap_out (victim->page);

    frame_free (victim->frame);
    frame = palloc_get_page (flags);
    ASSERT (frame != NULL);
  }
  ft_add_entry (frame, page, thread_current ()->pagedir);
  
  lock_release (&frame_alloc_lock);
  return frame;
}

void
frame_free (void *frame)
{
  lock_acquire (&frame_free_lock);

  struct frameTableEntry *fte = ft_get_entry (frame);
  pagedir_clear_page (fte->pagedir, fte->page);
  palloc_free_page (frame);
  ft_delete_entry (frame);

  lock_release (&frame_free_lock);
}

