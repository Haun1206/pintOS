#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include "threads/synch.h"
#include <stdint.h>
#include "threads/interrupt.h"
#ifdef VM
#include "vm/vm.h"
#endif


/* States in a thread's life cycle. */
enum thread_status {
	THREAD_RUNNING,     /* Running thread. */
	THREAD_READY,       /* Not running but ready to run. */
	THREAD_BLOCKED,     /* Waiting for an event to trigger. */
	THREAD_DYING        /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.
 *
 * Each thread structure is stored in its own 4 kB page.  The
 * thread structure itself sits at the very bottom of the page
 * (at offset 0).  The rest of the page is reserved for the
 * thread's kernel stack, which grows downward from the top of
 * the page (at offset 4 kB).  Here's an illustration:
 *
 *      4 kB +---------------------------------+
 *           |          kernel stack           |
 *           |                |                |
 *           |                |                |
 *           |                V                |
 *           |         grows downward          |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           +---------------------------------+
 *           |              magic              |
 *           |            intr_frame           |
 *           |                :                |
 *           |                :                |
 *           |               name              |
 *           |              status             |
 *      0 kB +---------------------------------+
 *
 * The upshot of this is twofold:
 *
 *    1. First, `struct thread' must not be allowed to grow too
 *       big.  If it does, then there will not be enough room for
 *       the kernel stack.  Our base `struct thread' is only a
 *       few bytes in size.  It probably should stay well under 1
 *       kB.
 *
 *    2. Second, kernel stacks must not be allowed to grow too
 *       large.  If a stack overflows, it will corrupt the thread
 *       state.  Thus, kernel functions should not allocate large
 *       structures or arrays as non-static local variables.  Use
 *       dynamic allocation with malloc() or palloc_get_page()
 *       instead.
 *
 * The first symptom of either of these problems will probably be
 * an assertion failure in thread_current(), which checks that
 * the `magic' member of the running thread's `struct thread' is
 * set to THREAD_MAGIC.  Stack overflow will normally change this
 * value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
 * the run queue (thread.c), or it can be an element in a
 * semaphore wait list (synch.c).  It can be used these two ways
 * only because they are mutually exclusive: only a thread in the
 * ready state is on the run queue, whereas only a thread in the
 * blocked state is on a semaphore wait list. */
struct thread {
	/* Owned by thread.c. */
	tid_t tid;                          /* Thread identifier. */
	enum thread_status status;          /* Thread state. */
	char name[16];                      /* Name (for debugging purposes). */
    int64_t wake_time; //This information stores the wakeup tick.
	int priority;                       /* Priority. */
    int original_priority;
    
    int nice;
    int recent_cpu;
	/* Shared between thread.c and synch.c. */
	struct list_elem elem;/* List element. */
    struct lock * want_lock;
    struct list donation;
    struct list_elem donation_elem;
    struct list_elem process_elem;
#ifdef USERPROG
	/* Owned by userprog/process.c. */
	uint64_t *pml4;                     /* Page map level 4 */
#endif
#ifdef VM
	/* Table for whole virtual memory owned by thread. */
	struct supplemental_page_table spt;
	uint64_t rsp;
#endif
	/*child list element*/
	struct list_elem child_elem;
	/*child list*/
	struct list child;

	/*Checks if the process's program memory is loaded */
	bool success_load;
	
	/*Checks if process in ended */
	bool process_exit;

	/*Exit semaphore*/
	struct semaphore exit_sema;

	/*Load Semaphore*/
	struct semaphore load_sema;

	/*wait semaphore */
	struct semaphore wait_sema;
	/*Indicating the next process descriptor */
	int next_fd;

	struct file **fd_table;

	struct thread * parent;

	/*Status when exiting */
	int status_exit;

	/*Current running file*/
	struct file *cur_file;

	/*semaphore for child_fork*/
	struct semaphore child_fork;

	/*Telling that there is forked thread*/
	int forked;
	/*Exit status of the child*/
	int child_status_exit;
	/* Owned by thread.c. */
	struct intr_frame tf;               /* Information for switching */
	unsigned magic;                     /* Detects stack overflow. */
};

struct lock co_lock;
/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void set_next_awake_tick(int64_t new_next);
int64_t get_next_awake_tick(void);
void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
bool thread_compare_waketime(struct list_elem * x, struct list_elem * y, void *aux);
bool thread_compare_priority(struct list_elem *x, struct list_elem *y, void*aux);
void thread_sleep(int64_t waking_tick);
void thread_awake(int64_t signal_tick);
void thread_unblock (struct thread *);
void swap_working(void);
struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);
void donate_priority(void);
void remove_lock(struct lock *lock);
void refresh_priority(void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void mlfqs_priority (struct thread *t);
void mlfqs_recent_cpu (struct thread *t);
void mlfqs_load_avg (void);
void mlfqs_increment(void);
void mlfqs_recalc (void);

int count_ready_threads(void);
void do_iret (struct intr_frame *tf);

#endif /* threads/thread.h */
