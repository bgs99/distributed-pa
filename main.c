#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> // fork
#include <sys/wait.h>
#include <memory.h>
#include <errno.h>

#include "ipc.h"
#include "common.h"
#include "pa1.h"


typedef struct {
    pid_t pid;
    local_id local_pid;
    int *pipe_rd;
    int *pipe_wr;
} dist_process;

int processes_total;

static const char *const log_pipe_opened =
        "Pipe (rd %3d, wr %3d) has OPENED\n";


int send(void *self, local_id dst, const Message *msg) {
    dist_process s = *(dist_process *) self;


    ssize_t c;
    if ((c = write(s.pipe_wr[dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len)) < 0) {
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

    ssize_t c;
    if ((c = read(s.pipe_rd[from], &msg->s_header, sizeof(MessageHeader))) < 0) {
        perror("read");
        return errno;
    }

    if ((c = read(s.pipe_rd[from], &msg->s_payload, msg->s_header.s_payload_len)) < 0) {
        perror("read");
        return errno;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    printf("PID %d\n", getpid());

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                processes_total = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p] [number of processes]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    processes_total++;

    dist_process dp[processes_total];


    for (int i = 0; i < processes_total; i++) {
        dist_process process = {
                .pipe_rd = malloc(processes_total * sizeof(int)),
                .pipe_wr = malloc(processes_total * sizeof(int)),
        };
        dp[i] = process;
    }

    int pipefd[2];
    FILE *pipe_log = fopen(pipes_log, "w");
    for (int i = 0; i < processes_total; i++) {
        for (int j = 0; j < processes_total; j++) {
            if (i == j) {
                dp[i].pipe_rd[j] = -1;
                dp[j].pipe_wr[j] = -1;
                continue;
            }
            pipe(pipefd);
            // check errno?
            fprintf(pipe_log, log_pipe_opened, pipefd[0], pipefd[1]);
            dp[i].pipe_wr[j] = pipefd[1];
            dp[j].pipe_rd[i] = pipefd[0];
        }
    }
    fclose(pipe_log);

    dp[0].local_pid = PARENT_ID;
    dp[0].pid = getpid();

    FILE *event_log = fopen(events_log, "w");
    for (local_id i = 1; i < processes_total; i++) {
        dp[i].local_pid = i;
        if (fork() == 0) {
            /* handle child process */

            dp[i].pid = getpid();
            // close some pipes here

            // 1

            Message msg = {
                    .s_header = {
                            .s_type = STARTED,
                            .s_magic = MESSAGE_MAGIC
                    },
            };

            sprintf(msg.s_payload, log_started_fmt, dp[i].local_pid, dp[i].pid, getppid());
            msg.s_header.s_payload_len = (uint16_t) strlen(msg.s_payload);

            fprintf(event_log, log_started_fmt, dp[i].local_pid, dp[i].pid, getppid());
            fflush(event_log);
            printf(log_started_fmt, dp[i].local_pid, dp[i].pid, getppid());

            send_multicast(&dp[i], &msg);

            memset(msg.s_payload, 0, msg.s_header.s_payload_len);
            for (local_id j = 1; j < processes_total; ++j) {
                if (j != i) {
                    receive(&dp[i], j, &msg);
//                    printf("Received in %d process %s", dp[i].local_pid, msg.s_payload);
                }
            }

            fprintf(event_log, log_received_all_started_fmt, dp[i].local_pid);
            fflush(event_log);
            printf(log_received_all_started_fmt, dp[i].local_pid);

            // 2

            // 3
            memset(msg.s_payload, 0, msg.s_header.s_payload_len);
            msg.s_header.s_type = DONE;
            sprintf(msg.s_payload, log_done_fmt, dp[i].local_pid);
            msg.s_header.s_payload_len = (uint16_t) strlen(msg.s_payload);

            fprintf(event_log, log_done_fmt, dp[i].local_pid);
            fflush(event_log);
            printf(log_done_fmt, dp[i].local_pid);

            send_multicast(&dp[i], &msg);

            memset(msg.s_payload, 0, msg.s_header.s_payload_len);
            for (local_id j = 1; j < processes_total; ++j) {
                if (j != i) {
                    receive(&dp[i], j, &msg);
//                    printf("Received in %d process %s", dp[i].local_pid, msg.s_payload);
                }
            }
            fprintf(event_log, log_received_all_done_fmt, dp[i].local_pid);
            fflush(event_log);
            printf(log_received_all_done_fmt, dp[i].local_pid);

            fclose(event_log);
            return 0;
        }
    }

    Message res_msg;
    for (local_id j = 1; j < processes_total; ++j) {
        receive(&dp[0], j, &res_msg);
    }
    fprintf(event_log, log_received_all_started_fmt, dp[0].local_pid);
    fflush(event_log);
    printf(log_received_all_started_fmt, dp[0].local_pid);

    memset(res_msg.s_payload, 0, res_msg.s_header.s_payload_len);
    for (local_id j = 1; j < processes_total; ++j) {
        receive(&dp[0], j, &res_msg);
    }
    fprintf(event_log, log_received_all_done_fmt, dp[0].local_pid);
    fflush(event_log);
    printf(log_received_all_done_fmt, dp[0].local_pid);

    fclose(event_log);
    sleep(1);
    printf("Number of processes %d", processes_total);
    return 0;
}
