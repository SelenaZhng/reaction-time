/*
 * realtime.h
 *
 * Nils Napp
 * Cornell University
 * All right reserved
 *
 * Jan 2024
 * Ithaca NY
 *
 * This file is part of the ECE3140/CS3420 offering for Spring 2024. If you are not part
 * of this class you should not have access to this file. Do not post, share, or otherwise
 * distribute this file. We will consider it an AI violation if you do.
 */

#ifndef __REALTIME_H__
#define __REALTIME_H__

typedef struct {
	unsigned int sec;
	unsigned int msec;
} realtime_t;

// The current time relative to process_start
volatile extern realtime_t current_time;

// The number of processes that have terminated before or after their deadline, respectively.
extern int process_deadline_met;
extern int process_deadline_miss;

/* Create a new realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_create(void (*f)(void), int n, realtime_t* start, realtime_t* deadline);

#endif /* __REALTIME_H_INCLUDED */
