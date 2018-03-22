/* In Multi Level Feedback Queue Scheduling, we need to calculate the following:
 * priority, recent_cpu, and load_avg.
 * We have thus made a new file to help with fixed-point number calculations.
 * We use 17.14 format. */

/*
#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif
*/
#include "lib/stdint.h"

#include "threads/fixed_point_calc.h"

static int f = 1<<14;/* static int f = 2^14 in 17.14 format */

fp_t
convert_int_to_fp (int n)
{
  return n * f;
}

int
convert_fp_to_int_trunc (fp_t x)
{
  return x / f;
}

int
convert_fp_to_int_round (fp_t x)
{
  if (x >= 0) return (x + f/2)/f;
  return (x - f/2)/f;
}

fp_t
add_fp_and_fp (fp_t x, fp_t y)
{
  return x + y;
}

fp_t
subtract_fp_from_fp (fp_t x, fp_t y)
{
  return x - y;
}

fp_t
add_fp_and_int (fp_t x, int n)
{
  return x + n * f;
}

fp_t
subtract_int_from_fp (int n, fp_t x)
{
  return x - n * f;
}

fp_t
multiply_fp_by_fp (fp_t x, fp_t y)
{
  return ((int64_t) x) * y / f;
}

fp_t
multiply_fp_by_int (fp_t x, int n)
{
  return x * n;
}

fp_t
divide_fp_by_fp (fp_t x, fp_t y)
{
  return ((int64_t) x) * f / y;
}

fp_t
divide_fp_by_int (fp_t x, int n)
{
  return x / n;
}


