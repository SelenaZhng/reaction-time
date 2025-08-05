/* process.c
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

#include <stdlib.h>
#include "3140_concur.h"
#include "realtime.h"

process_queue_t rt_ready_queue = {NULL};
process_queue_t rt_non_ready_queue = {NULL};

volatile realtime_t current_time = {0, 0};

int process_deadline_met = 0;
int process_deadline_miss = 0;

int is_geq_time(realtime_t t1, realtime_t t2) {
    return (t1.sec > t2.sec) || (t1.sec == t2.sec && t1.msec >= t2.msec);
}

int is_greater_time(realtime_t t1, realtime_t t2) {
    return (t1.sec > t2.sec) || (t1.sec == t2.sec && t1.msec > t2.msec);
}

void insert_by_deadline(process_queue_t *q, process_t *proc) {
    process_t *prev = NULL, *curr = q->head;
    while (curr && is_geq_time(curr->deadline, proc->deadline)) {
        prev = curr;
        curr = curr->next;
    }
    proc->next = curr;
    if (prev) prev->next = proc;
    else q->head = proc;
}

/* Stack memory cleanup */
static void process_free(process_t *proc) {
    process_stack_free(proc->orig_sp, proc->n);
    free(proc);
}

/* Starts up the concurrent execution */
void process_start(void) {
	current_time.sec = 0;
	current_time.msec = 0;

    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR = 0;
    // scheduling timer
    PIT->CHANNEL[0].LDVAL = 150000;
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK;
    // real-time clock
    PIT->CHANNEL[1].LDVAL = 15000;
    PIT->CHANNEL[1].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

    NVIC_EnableIRQ(PIT_IRQn);

	if (is_empty(&process_queue) && is_empty(&rt_non_ready_queue)) return;

    process_begin();
}

/* Create a new non-realtime process */
int process_create(void (*f)(void), int n) {
    unsigned int *sp = process_stack_init(f, n);
    if (!sp) return -2;

    process_t *proc = malloc(sizeof(process_t));
    if (!proc) {
        process_stack_free(sp, n);
        return -1;
    }

    proc->sp = proc->orig_sp = sp;
    proc->n = n;
    proc->is_realtime = 0;
    proc->next = NULL;

    enqueue(proc, &process_queue);
    return 0;
}

/* Create a new realtime process */
int process_rt_create(void (*f)(void), int n, realtime_t *start, realtime_t *deadline) {
    unsigned int *sp = process_stack_init(f, n);
    if (!sp) return -2;

    process_t *proc = malloc(sizeof(process_t));
    if (!proc) {
        process_stack_free(sp, n);
        return -1;
    }

    proc->sp = proc->orig_sp = sp;
    proc->n = n;
    proc->start_time = *start;
    proc->deadline.sec = start->sec + deadline->sec;
    proc->deadline.msec = start->msec + deadline->msec;
    if (proc->deadline.msec >= 1000) {
        proc->deadline.sec += proc->deadline.msec / 1000;
        proc->deadline.msec %= 1000;
    }

    proc->is_realtime = 1;
    proc->next = NULL;

    enqueue(proc, &rt_non_ready_queue);
    return 0;
}

/* Idle process in case no runnable processes exist */
void idle_proc(void) {
    while (1);
}

/* Scheduler logic */
unsigned int *process_select(unsigned int *cursp) {
    if (cursp) {
        if (current_process_p && current_process_p->is_realtime)
            return cursp;

        current_process_p->sp = cursp;
        enqueue(current_process_p, &process_queue);
    } else if (current_process_p) {
        if (current_process_p->is_realtime) {
            if (is_greater_time(current_time, current_process_p->deadline))
                process_deadline_miss++;
            else
                process_deadline_met++;
        }
        process_free(current_process_p);
    }

    // Promote ready RT processes
    process_t *curr = rt_non_ready_queue.head;
    while (curr) {
        process_t *next = curr->next;
        if (is_geq_time(current_time, curr->start_time)) {
            process_t *to_promote = dequeue(&rt_non_ready_queue);
            insert_by_deadline(&rt_ready_queue, to_promote);
        }
        curr = next;
    }

    if (is_empty(&process_queue) && is_empty(&rt_ready_queue) && !is_empty(&rt_non_ready_queue)) {
        process_create(idle_proc, 20);
    }

    current_process_p = !is_empty(&rt_ready_queue) ? dequeue(&rt_ready_queue)
                                                   : dequeue(&process_queue);

    return current_process_p ? current_process_p->sp : NULL;
}

void PIT1_Service(void){
	PIT->CHANNEL[1].TFLG |= PIT_TFLG_TIF_MASK;

	current_time.msec += 1;
	if (current_time.msec >= 1000){
		current_time.sec += 1;
		current_time.msec -= 1000;
	}

	// Reload PIT1 counter
	PIT->CHANNEL[1].LDVAL = 15000;
}



