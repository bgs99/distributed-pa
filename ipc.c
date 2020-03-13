//
// Created by edem on 13.03.20.
//

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "distributed.h"

int send(void *self, local_id dst, const Message *msg) {
    dist_process s = *(dist_process *) self;

    if ((write(s.pipe_wr[dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len)) < 0) {
        perror("write");
        return errno;
    }
    return 0;
}

int send_multicast(void *self, const Message *msg) {
    dist_process p = *(dist_process *) self;
    for (local_id i = 0; i < processes_total; ++i) {
        if (i != p.local_pid) {
            int res = send(self, i, msg);
            if (res != 0) {
                return res;
            }
        }
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    dist_process s = *(dist_process *) self;

    if ((read(s.pipe_rd[from], &msg->s_header, sizeof(MessageHeader))) < 0) {
        perror("read");
        return errno;
    }

    if ((read(s.pipe_rd[from], &msg->s_payload, msg->s_header.s_payload_len)) < 0) {
        perror("read");
        return errno;
    }

    return 0;
}


int receive_any(void * self, Message * msg) {
    return EOPNOTSUPP;
}

