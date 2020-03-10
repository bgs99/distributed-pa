#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> // fork
#include <sys/wait.h>
#include <memory.h>

#include "ipc.h"
#include "common.h"
#include "pa1.h"


/*
    int wc_pid = fork();
    if (wc_pid == 0) {
        close(pipefd_wr);
        ensure_correct(" ");
        dup2(pipefd_rd, STDIN_FILENO);
        ensure_correct(" ");
        execlp("wc", "wc", "-c", (char*)NULL);
        ensure_correct(" ");
    }
    else {
        close(pipefd_rd);
        ensure_correct(" ");
        return pipe_file(file, pipefd_wr);
    }
 */
typedef struct {
    pid_t pid;
    local_id local_pid;
    int *pipe_rd;
    int *pipe_wd;
    timestamp_t runtime;
} dist_process;

static const char *const log_pipe_opened =
        "Pipe (rd %3d, wr %3d) has OPENED\n";

/*
 *     printf("------------- \n");
    printf("local_pid %d\n", s->local_pid);

    for (int i = 0; i < 11; ++i) {
        printf("pipe rd %d\n", s->pipe_rd[i]);
        printf("pipe wd %d\n", s->pipe_wd[i]);

    }
    printf("------------- \n");

 */

int send(void *self, local_id dst, const Message *msg) {
    dist_process s = *(dist_process *) self;
    printf("\tmsg payload length = %d\n", msg->s_header.s_payload_len);
    printf("\tsizeof MessageHeader = %d\n", (int) sizeof(MessageHeader));
    printf("\tsizeof Message = %d\n", (int) sizeof(Message));
    printf("\tsizeof msg = %d\n", (int) sizeof(msg));
    printf("\tsend to  %d\n", s.pipe_wd[dst]);
//    write(5, &msg->s_header, sizeof(MessageHeader));
//    write(5, &msg->s_payload, msg->s_header.s_payload_len);
    write(s.pipe_wd[dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);

    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    dist_process s = *(dist_process *) self;

    printf("\treceive from %d\n", s.pipe_rd[from]);
    printf("\treceive msg length %d\n", (int) sizeof(Message));

    read(s.pipe_rd[from], msg, sizeof(Message));

//    read(s.pipe_rd[from], &msg->s_header, sizeof(MessageHeader));
//    read(s.pipe_rd[from], &msg->s_payload, msg->s_header.s_payload_len);

    return 0;
}

int main(int argc, char *argv[]) {
    int n = 0;
    int opt;
    printf("PID %d\n", getpid());

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                n = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p] [number of processes]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    dist_process dp[n + 1];

    for (int i = 0; i < n + 1; i++) {
        dist_process process = {
                .pipe_rd = malloc((n + 1) * sizeof(int)),
                .pipe_wd = malloc((n + 1) * sizeof(int)),
        };
        dp[i] = process;
    }

    int pipefd[2];
    FILE *pipe_log = fopen(pipes_log, "w");
    for (int i = 0; i < n + 1; i++) {
        for (int j = 0; j < n + 1; j++) {
            if (i == j) {
                dp[i].pipe_rd[j] = -1;
                dp[j].pipe_wd[j] = -1;
                continue;
            }
            pipe(pipefd);
            // check errno?
            fprintf(pipe_log, log_pipe_opened, pipefd[0], pipefd[1]);
            dp[i].pipe_wd[j] = pipefd[1];
            dp[j].pipe_rd[i] = pipefd[0];
        }
    }
    fclose(pipe_log);

    dp[0].local_pid = PARENT_ID;
    dp[0].pid = getpid();

    Message msg = {
            .s_header = {
                    .s_type = STARTED,
                    .s_magic = MESSAGE_MAGIC
            },
    };

    sprintf(msg.s_payload, log_started_fmt, dp[0].local_pid, dp[0].pid, getppid());
    msg.s_header.s_payload_len = (uint16_t) strlen(msg.s_payload);
    send(&dp[0], 1, &msg);

        for (local_id i = 1; i < n + 1; i++) {
        dp[i].local_pid = i;
        if (fork() == 0) {
            // close some pipes here
            printf("dp %d\n", dp[i].local_pid);
            Message *res_msg = malloc(sizeof(Message));
            if (dp[i].local_pid == 1) {
                printf("inside if \n");
                receive(&dp[i], 0, res_msg);
                printf("res_msg->s_header.s_payload_len %d\n", res_msg->s_header.s_payload_len);
                printf("res_msg->s_header.s_type %d\n", res_msg->s_header.s_type);
                printf("msg %s\n", res_msg->s_payload);
            }
            /* handle child process */
            exit(getpid());
        }
    }
    for (int i = 0; i < n + 1; i++) {
//        send(&dp[i], 1, NULL);
    }
    sleep(100);
    printf("Number of processes %d", n);
    return 0;
}
