#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void usage(const char *a) {
    fprintf(stderr, "Usage: %s <cmd> [args]\n", a);
    exit(1);
}

static double d(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Child PID: %d\n", pid);
    if (WIFEXITED(status)) {
        printf("Exit code: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Terminated by signal: %d\n", WTERMSIG(status));
    }

    printf("Elapsed time: %.6f sec\n", d(start, end));

    return 0;
}
