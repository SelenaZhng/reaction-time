/* 3140_concur.h
 *
 * Nils Napp
 * Cornell University
 * All right reserved
 *
 * Jan 2024
 * Ithaca NY
 *
 * This file is part of the ECE3140/CS3420 offering for Spring 2024. If you
 * are not part of this class you should not have access to this file. Do not
 * post, share, or otherwise distribute this file. We will consider it an AI
 * violation if you do. If you somehow get this code and you are NOT enrolled
 * the Spring 2024 version of ECE3140 please contact the course staff
 * immediately and describe how you found it.
 */

#ifndef __3140_CONCUR_H__
#define __3140_CONCUR_H__

#include <stdlib.h>
#include <MKL46Z4.h>
#include "process.h"

/*
 * The process queue type is just a pointer to the head element
 * This will use the definition of process_t that you should define in process.c
 *
 */
typedef struct process_queue{
	process_t * head;
} process_queue_t;


/*
 *   Global variables to keep track of the processes in the system.
 *   current_process_p must be NULL if no process is running,
 *   otherwise it must point to the process_t of the currently running process.
 *   The variables are extern so that everything that includes "3140_concur.h" can
 *   refer to them, but they are defined somewhere else (in 3140_concur.c).
 *
 *   Note the c-convention of including a p in the name of pointers.
 */

extern process_t * current_process_p;
extern process_queue_t  process_queue;


/*------------------------------------------------------------------------
   THE FOLLOWING FUNCTIONS MUST BE CREATED IN process.c.
------------------------------------------------------------------------*/

/* 
 *  Called by the runtime system to select another process:
 *  "cursp" = the stack pointer for the currently running process
 *  cursp will be NULL when first starting the scheduler, and when a process terminates
 *  Return the stack pointer for the new process to run, or NULL to exit the scheduler.
 */
unsigned int * process_select (unsigned int * cursp);

/* Starts up the concurrent execution */
void process_start (void);

/* Create a new process. Return -1 if creation failed */
int process_create (void (*f)(void), int n);

/*---------------------------------------------------------------------
  
You may use the following functions that we have provided

-----------------------------------------------------------------------*/

/*
 * Functions to interact with the process queue
 *
 * dequeue(process_queue_t * proc_queue_p)
 *
 * Returns the address of the first element and removes it from the queue.
 * It returns NULL if the queue is empty.
 *
 *
 * enqueue (process_t * process,  process_queue_t * proc_queue_p)
 *
 * Adds process to the end of proc_queue.
 *
 * You should call these with the globally defined process_queue_p defined above.
 * For example:
 *
 * To add a process pointed to by mypcb_p to the end of the queue call:
 *
 *     enqueue(mypcb_p,&process_queue)
 *
 * To remove the first element of the queue call:
 *
 *      process_t * old_head_p;
 *      hold_head_p = dequeue(&process_queue);
 *
 * To check if a queue is empty:
 *
 * 		if ( is_empty(&process_queue) ){
 * 		  // Do something
 * 		}
 *
 */
process_t * dequeue(process_queue_t * proc_queue_p);
void enqueue (process_t * process,  process_queue_t * proc_queue_p);
int is_empty(process_queue_t * proc_queue_p);

/*
 * This function is new and was not needed in Lab 3.
 * This function triggers a context switch by causing and exception
 * and entering the PIT IRQ handler. This way, you can initiate a context
 * switch from your code even if the PIT is not fired or the process has terminated.
 *
 *  Implemented in 3140.s
 */
extern void process_blocked (void);

/*
 * This function is called by user code indirectly when the process
 * terminates. You should not need to call this function yourself.
 *
 * Implemented in 3140.s
 * Used in 3140_concur.c
 */
extern void process_terminated (void);


/* 
 *  This function can ONLY BE CALLED if interrupts are disabled. It
 *  does not modify interrupt flags.
 *
 *	 Allocates a stack for a process, and sets up the stack's initial state
 *
 *	 Implemented in 3140_concur.c
 */
unsigned int * process_stack_init (void (*f)(void), int n);


/* 
 *  This function can ONLY BE CALLED if interrupts are disabled. It
 *  does not modify interrupt flags. You'll want to use this inside process select, and
 *  interrupts should be turned off once execution is given to process select.
 *
 *	 Frees a stack allocated in process_init. Must be called with the same value
 *	 of n as process_init
 *
 *	 Implemented in 3140_concur.c
 */
void process_stack_free (unsigned int *sp, int n);

/*
 * This function starts the concurrency by using the timer interrupt
 * context switch routine to call the first ready process.
 *
 * The function also gracefully exits once the process_select()
 * function returns 0.
 *
 *	Implemented in 3140.s
 */
extern void process_begin (void);

/*
 * Place holder function for reporting code addresses of where deadlock occurs.
 * This is only used in the EC, and not necessary to complete this lab.
 */
void report(unsigned int *);

#endif
