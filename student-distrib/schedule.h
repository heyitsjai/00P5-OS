#ifndef SCHEDULE_H__
#define SCHEDULE_H__
#include "paging.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "systemcalls.h"
#include "x86_desc.h"

#define MAX_TERMINALS 3
void schedule_initialization();
void schedule_handling();

typedef struct {
    volatile int current_running_process_count;
    volatile int next_terminal_in_schedule;
    volatile int schedule_activated;
}sched_t;

sched_t active_schedule;

#endif
