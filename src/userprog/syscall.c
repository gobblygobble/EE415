#include "userprog/syscall.h"
#include "userprog/file.h"
#include "userprog/filesys.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/sync.h"

static void syscall_handler (struct intr_frame *);
static struct intr_frame *f_

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  /* retrieve syscall number from intr_frame */
  f_ = f;
  int syscall_num = *((int *)(f->esp));
  uint32_t arg0, arg1, arg2;

  switch (syscall_num)
  {
    case SYS_HALT:
      halt ();
      break;

    case SYS_EXIT:
      exit ();
      break;

    case SYS_EXEC:
      exec ();
      break;

    case SYS_WAIT:
      wait ();
      break;

    case SYS_CREATE:
      create ();
      break;

    case SYS_REMOVE:
      remove ();
      break;

    case SYS_OPEN:
      open ();
      break;

    case SYS_FILESIZE:
      filesize ();
      break;

    case SYS_READ:
      read ();
      break;

    case SYS_WRITE:
      write ();
      break;

    case SYS_SEEK:
      seek ();
      break;

    case SYS_TELL:
      tell ();
      break;

    case SYS_CLOSE:
      close ();
      break;
  }

  thread_exit ();
}

void
halt (void)
{
  shutdown_power_off ();
}

void
exit (int status)
{
  struct thread *t = pg_round_down (f_->esp);
  uint32_t *pd = t->pagedir;
  if (pd != NULL)
  {
    t->pagedir = NULL;
    pagedir_activate (NULL);
    pagedir_destroy (pd);
  }
  (f_->eax) = status;
}

pid_t
exec (const char *cmd_line)
{
  ;
}

int
wait (pid_t pid)
{
  ;
}

bool
create (const char *file, unsigned initial_size)
{
  ;
}

bool
remove (const char *file)
{
  bool success = filesys_remove (file);
}

int
open (const char *file)
{
  struct file *opened_file = filesys_open (file);
  struct thread *t = pg_round_down (f_->esp);
  int fd;

  /* find empty entry in fd_table */
  for (fd = 2; fd < MAX_FD; fd++) {
    if (t->fd_table[fd] == NULL) break;
  }
  if (fd == MAX_FD) {
    /* fd_table is full */
    (f_->eax) = -1;
  }
  else {
    t->fd_table[fd] = opened_file;
    (f_->eax) = fd;
  }

}

int
filesize (int fd)
{
  struct thread *t = pg_round_down (f_->esp);
  struct file *opened_file = t->fd_table[fd];

  int length = ((opened_file->inode)->data).length;
  (f_->eax) = length;
}

int
read (int fd, void *buffer, unsigned size)
{
  struct lock read_lock;
  struct file *file;
  struct thread *t = pg_round_down (f_->esp);
  int read_cnt = 0;

  if (fd == 0) {
    lock_acquire (&read_lock);
    while (read_cnt <= size) {
      /* read key by input_getc() and write it into buffer at appropriate position */
      *(buffer + read_cnt++) = input_getc ();
    }
    lock_release (&read_lock);
    (f_->eax) = read_cnt;
    return;
  }

  /* get file from fd */
  file = t->fd_table[fd];

  lock_acquire (&read_lock);
  (f_->eax) = file_read (file, buffer, size);
  lock_release (&read_lock);
}

int
write (int fd, const void *buffer, unsigned size)
{
  struct lock write_lock;
  struct file *file;
  struct thread *t = pg_round_down (f_->esp);

  if (fd == 1) {
    lock_acquire (&write_lock);
    putbuf (buffer, size);
    lock_release (&write_lock);
    (f_->eax) = size;
    return;
  }

  /* get file from fd */
  file = t->fd_table[fd];

  /* acquire write_lock */
  lock_acquire (&write_lock);
  (f_->eax) = file_write (file, buffer, size);
  lock_release (&write_lock);
}

void
seek (int fd, unsigned position)
{
  struct thread *t = pg_round_down (f_->esp);
  struct file *opened_file = t->fd_table[fd];

  opened_file->pos = (int32_t)position;
}

unsigned
tell (int fd)
{
  struct thread *t = pg_round_down (f_->esp);
  struct file *opened_file = t->fd_table[fd];
 
  int next = opened_file->pos + 1;

  ASSERT (next >= 0);

  unsigned next_u = (unsigned) next;
  (f_->eax) = next_u;
}

void
close (int fd)
{
  struct thread *t = pg_round_down (f_->esp);
  struct file *opened_file = t->fd_table[fd];  

  if (fd == 0 || fd == 1) {
    return;
  }

  file_close (opened_file);
  t->fd_table[fd] = NULL;
}


