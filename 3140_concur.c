/* 3140_concur.c
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

#include "3140_concur.h"
#include <stdlib.h>

/*
  State layout:

  .-----------------.
  |     xPSR   	    | <--- status register
  |-----------------|
  |      PC         | <--- starting point of the process's function    
  |-----------------|
  |      LR         | <--- process_terminated
  |-----------------|
  |      R12        |
  |-----------------|
  |    R3 - R0      |
  |-----------------|
  |   0xFFFFFFF9    | <--- exception return value (pin in LR by interrupt HW)
  |-----------------|
  |    R4 - R7      | <----
  |-----------------|	   |--- The order of these two is "flipped" from the normal veradic 
  |    R8 - R11     | <----     order due to the low-reg limitation of PUSH/POP in ARMv6-M
  |-----------------|
  |    PIT State    |
  |-----------------|


  Context requires 18 32-bit entries on the stack.
 */


/*
 * Global variables for managing process queue
 */
process_t * current_process_p = NULL;
process_queue_t process_queue = {NULL};

void enqueue(process_t * pcb_p, process_queue_t * queue_p){
	process_t * tmp_p;

	//make sure that the process is not pointing to a next queue
	pcb_p->next = NULL;

	if (queue_p->head == NULL){
		//queue is empty
		//make the new process the head
		queue_p->head = pcb_p;
	}else{
		//queue is not empty
		//loop until tmp_p points to the last element
		tmp_p=queue_p->head;
		while(!(tmp_p->next == NULL)){
			tmp_p=tmp_p->next;
		}
		tmp_p->next = pcb_p;
	}
}


process_t *  dequeue(process_queue_t * queue_p){
	//variable to keep track of the element we are
	//removing from the queue
	process_t * tmp_p;

	if (queue_p->head == NULL){
		//queue is empty
		//return NULL
		return NULL;

	}else{
		//queue is not empty
		//put head of queue to next node
		tmp_p=queue_p->head;
		queue_p->head = tmp_p->next;

		//remove reference to list from old head
		tmp_p->next = NULL;
		return tmp_p;
	}
}

int is_empty(process_queue_t * proc_queue_p){
	return (proc_queue_p->head == NULL);
}

/*------------------------------------------------------------------------
 *
 *   process_stack_init --
 *
 *   Allocate and initialize a stack for a process
 *
 *------------------------------------------------------------------------
 */

unsigned int * process_stack_init (void (*f)(void), int n)
{

  unsigned int *sp;	/* Pointer to process stack (allocated in heap) */ 
	
  int i;

  /* in reality, there are 18 more slots needed for stored context */
  n += 18;


  /* Allocate space for the process's stack */
  sp = malloc(n*sizeof(int));
		 
  if (sp == NULL) { return NULL; }	/* Allocation failed */
  
  /* Initialize the stack to all zeros   */
  /* Note: Could just use calloc instead */ 
  for (i=0; i < n; i++) {
  	sp[i] = 0;
  }
  
  sp[n-1] = 0x01000000;          			   // xPSR
  sp[n-2] = (unsigned int) f;    			   // PC -> function that was passed in
  sp[n-3] = (unsigned int) process_terminated; // LR "return" to process terminated
  sp[n-9] = 0xFFFFFFF9; // EXC_RETURN value, returns to thread mode
  sp[n-18] = 0x3;       // Enable scheduling timer and interrupt (This would be more readable with masks!)


  return &(sp[n-18]);

}

/*------------------------------------------------------------------------
 *
 *  process_stack_free --
 *
 *  Free a process stack allocated by process_stack_init. Call this with the SP
 *  which was returned by process_stack_init, and the stack size which was passed
 *  to process_stack_init
 *
 *------------------------------------------------------------------------
 */
void process_stack_free(unsigned int *sp, int n)
{
	// process_init returned a pointer to the top of the stack, which is near
	// the end of the allocated region. We need to recover the pointer returned
	// by malloc
	unsigned int *stack_base = sp - n;
	free(stack_base);
}
