#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include <stdio.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include <stdbool.h>
#include "devices/shutdown.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
static struct lock filelock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&filelock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  /* retrieve syscall number from intr_frame */
  int syscall_num = *((int *)(f->esp));
  uint32_t arg0, arg1, arg2;

  switch (syscall_num)
  {
    case SYS_HALT:
      halt ();
      break;

    case SYS_EXIT:
      arg0 = *(uint32_t *)(f->esp + 4);
      exit ((int)arg0);
      (f->eax) = (int)arg0;
      break;

    case SYS_EXEC:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = exec ((char *)arg0);
      break;

    case SYS_WAIT:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = wait ((pid_t)arg0);
      break;

    case SYS_CREATE:
      arg0 = *(uint32_t *)(f->esp + 4);
      arg1 = *(uint32_t *)(f->esp + 8);
      (f->eax) = create ((char *)arg0, (unsigned)arg1);
      break;

    case SYS_REMOVE:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = remove ((char *)arg0);
      break;

    case SYS_OPEN:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = open ((char *)arg0);
      break;

    case SYS_FILESIZE:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = filesize ((int)arg0);
      break;

    case SYS_READ:
      arg0 = *(uint32_t *)(f->esp + 4);
      arg1 = *(uint32_t *)(f->esp + 8);
      arg2 = *(uint32_t *)(f->esp + 12);
      (f->eax) = read ((int)arg0, (void *)arg1, (unsigned)arg2);
      break;

    case SYS_WRITE:
      arg0 = *(uint32_t *)(f->esp + 4);
      arg1 = *(uint32_t *)(f->esp + 8);
      arg2 = *(uint32_t *)(f->esp + 12);
      (f->eax) = write ((int)arg0, (void *)arg1, (unsigned)arg2);
      break;

    case SYS_SEEK:  
      arg0 = *(uint32_t *)(f->esp + 4);
      arg1 = *(uint32_t *)(f->esp + 8);
      seek ((int)arg0, (unsigned)arg1);
      break;

    case SYS_TELL:
      arg0 = *(uint32_t *)(f->esp + 4);
      (f->eax) = tell ((int)arg0);
      break;

    case SYS_CLOSE:
      arg0 = *(uint32_t *)(f->esp + 4);
      close ((int)arg0);
      break;
  }
}

void
halt (void)
{
  shutdown_power_off ();
}

void
exit (int status)
{
  struct thread *cur = thread_current ();

  /* wait until parent info is set */
  sema_down (&cur->parent_sema);

  struct child *ch = &cur->child_info;

  ASSERT (ch->child_tid == cur->tid);

  ch->status = status;
  sema_up (&ch->sema);

  thread_exit ();
}

pid_t
exec (const char *cmd_line)
{
  //struct thread *t = pg_round_down (f_->esp);
  tid_t child = process_execute (cmd_line);

  return (pid_t)child;
}

int
wait (pid_t pid)
{
  return process_wait (pid);
}

bool
create (const char *file, unsigned initial_size)
{
  lock_acquire (&filelock);
  bool success = filesys_create (file, initial_size);
  lock_release (&filelock);
  return success;
}

bool
remove (const char *file)
{
  lock_acquire (&filelock);
  bool success = filesys_remove (file);
  lock_release (&filelock);
  return success;
}

int
open (const char *file)
{
  lock_acquire (&filelock);
  struct file *opened_file = filesys_open (file);
  lock_release (&filelock);
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  int fd;

  /* find empty entry in fd_table */
  for (fd = 2; fd < MAX_FD; fd++) {
    if (t->fd_table[fd] == NULL) break;
  }
  if (fd == MAX_FD) {
    /* fd_table is full */
    return -1;
  }
  else {
    t->fd_table[fd] = opened_file;
    return fd;
  }
}

int
filesize (int fd)
{
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  struct file *opened_file = t->fd_table[fd];
  int length;

  lock_acquire (&filelock);
  length = file_length (opened_file);
  lock_release (&filelock);

  return length;
}

int
read (int fd, void *buffer, unsigned size)
{
  struct file *file;
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  unsigned read_cnt = 0;

  lock_acquire (&filelock);
  if (fd == 0) {
    while (read_cnt <= size) {
      /* read key by input_getc() and write it into buffer at appropriate position */
      *(char *)(buffer + read_cnt++) = input_getc ();
    }
    lock_release (&filelock);
    return read_cnt;
  }

  /* get file from fd */
  file = t->fd_table[fd];
  read_cnt = file_read (file, buffer, size);
  lock_release (&filelock);
  return (int)read_cnt;
}

int
write (int fd, const void *buffer, unsigned size)
{
  struct file *file;
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  int write_cnt = size;
  
  lock_acquire (&filelock);
  if (fd == 1) {
    putbuf (buffer, size);
    lock_release (&filelock);
    return write_cnt;
  }

  /* get file from fd */
  file = t->fd_table[fd];

  write_cnt = file_write (file, buffer, size);
  lock_release (&filelock);
  return write_cnt;
}

void
seek (int fd, unsigned position)
{
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  struct file *opened_file = t->fd_table[fd];
  
  lock_acquire (&filelock);
  file_seek (opened_file, position);
  lock_release (&filelock);
}

unsigned
tell (int fd)
{
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  struct file *opened_file = t->fd_table[fd];
  int next;
  
  lock_acquire (&filelock);
  next = file_tell (opened_file);
  lock_release (&filelock);

  return (unsigned) next;
}

void
close (int fd)
{
  //struct thread *t = pg_round_down (f_->esp);
  struct thread *t = thread_current ();
  struct file *opened_file = t->fd_table[fd];  

  if (fd == 0 || fd == 1) {
    return;
  }
  lock_acquire (&filelock);
  file_close (opened_file);
  lock_release (&filelock);
  t->fd_table[fd] = NULL;
}

