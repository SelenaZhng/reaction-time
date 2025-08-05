/*
 * process.h
 *
 * Nils Napp
 * Cornell University
 * All rights reserved
 *
 * Jan 2024
 * Ithaca NY
 *
 * This file is part of the ECE3140/CS3420 offering for Spring 2024. If you are not part
 * of this class you should not have access to this file. Do not post, share, or otherwise
 * distribute this file. We will consider it an AI violation if you do.
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "realtime.h"

struct process_state {
    unsigned int *sp;
    unsigned int *orig_sp;
    int n;
    struct process_state *next;

    int is_realtime;               // 1 if real-time, 0 if not
    realtime_t start_time;        // When the process is eligible to start
    realtime_t deadline;          // Absolute deadline for EDF scheduling
};

typedef struct process_state process_t;

#endif /* PROCESS_H_ */

