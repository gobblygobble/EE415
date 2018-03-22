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
#include "lib/stdint.h"
*/

typedef int fp_t;

fp_t convert_int_to_fp (int);
int convert_fp_to_int_trunc (fp_t);
int convert_fp_to_int_round (fp_t);
fp_t add_fp_and_fp (fp_t, fp_t);
fp_t subtract_fp_from_fp (fp_t, fp_t);
fp_t add_fp_and_int (fp_t, int);
fp_t subtract_int_from_fp (int, fp_t);
fp_t multiply_fp_by_fp (fp_t, fp_t);
fp_t multiply_fp_by_int (fp_t, int);
fp_t divide_fp_by_fp (fp_t, fp_t);
fp_t divide_fp_by_int (fp_t, int);
