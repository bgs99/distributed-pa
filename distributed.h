//
// Created by edem on 07.03.20.
//

#pragma once
#include <sys/types.h>
#include "ipc.h"

typedef struct {
    pid_t pid;
    local_id local_pid;
    int *pipe_rd;
    int *pipe_wr;
} dist_process;

int processes_total;
