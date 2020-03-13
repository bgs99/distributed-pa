//
// Created by edem on 13.03.20.
//

#ifndef DISTRIBUTEDOMPUTING_PA1_LOGGER_H
#define DISTRIBUTEDOMPUTING_PA1_LOGGER_H

#include <stdio.h>
#include "pa1.h"
static const char *const log_pipe_opened =
        "Pipe (rd %3d, wr %3d) has OPENED\n";

FILE *event_log;

void log_started(local_id local_pid) {
    fprintf(event_log, log_started_fmt, local_pid, getpid(), getppid());
    fflush(event_log);
    printf(log_started_fmt, local_pid, getpid(), getppid());
}

void log_received_all_started(local_id local_pid) {
    fprintf(event_log, log_received_all_started_fmt, local_pid);
    fflush(event_log);
    printf(log_received_all_started_fmt, local_pid);
}

void log_done(local_id local_pid) {
    fprintf(event_log, log_done_fmt, local_pid);
    fflush(event_log);
    printf(log_done_fmt, local_pid);
}

void log_received_all_done(local_id local_pid) {
    fprintf(event_log, log_received_all_done_fmt, local_pid);
    fflush(event_log);
    printf(log_received_all_done_fmt, local_pid);
}

#endif //DISTRIBUTEDOMPUTING_PA1_LOGGER_H
