//
// Created by edem on 07.03.20.
//

#ifndef DISTRIBUTEDOMPUTING_PA1_TEMP_H
#define DISTRIBUTEDOMPUTING_PA1_TEMP_H

#include <unistd.h>
#include "ipc.h"

typedef struct {
    pid_t pid;
    local_id local_pid;
    int *pipe_rd;
    int *pipe_wr;
} dist_process;

int processes_total;

#endif //DISTRIBUTEDOMPUTING_PA1_TEMP_H
