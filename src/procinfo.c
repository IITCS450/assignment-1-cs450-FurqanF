#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void usage(const char *a) {
    fprintf(stderr, "Usage: %s <pid>\n", a);
    exit(1);
}

static int isnum(const char *s) {
    for (; *s; s++) if (!isdigit(*s)) return 0;
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2 || !isnum(argv[1])) usage(argv[0]);

    int pid = atoi(argv[1]);
    char stat_path[256], status_path[256], cmdline_path[256];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);

    FILE *fstat = fopen(stat_path, "r");
    if (!fstat) { perror("fopen stat"); return 1; }

    char comm[256], state;
    int ppid;
    long utime, stime;

    fscanf(fstat, "%*d (%255[^)]) %c %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld",
           comm, &state, &ppid, &utime, &stime);
    fclose(fstat);

    FILE *fstatus = fopen(status_path, "r");
    if (!fstatus) { perror("fopen status"); return 1; }

    char line[256];
    long vmrss = 0;
    while (fgets(line, sizeof(line), fstatus)) {
        if (sscanf(line, "VmRSS: %ld kB", &vmrss) == 1) break;
    }
    fclose(fstatus);

    FILE *fcmd = fopen(cmdline_path, "r");
    char cmdline[1024] = "";
    if (fcmd) {
        size_t n = fread(cmdline, 1, sizeof(cmdline) - 1, fcmd);
        fclose(fcmd);
        for (size_t i = 0; i < n; i++) if (cmdline[i] == '\0') cmdline[i] = ' ';
        cmdline[n] = '\0';
    } else {
        strcpy(cmdline, comm);
    }

    printf("PID: %d\n", pid);
    printf("State: %c\n", state);
    printf("Parent PID: %d\n", ppid);
    printf("Command line: %s\n", cmdline);
    printf("CPU time: %.2f sec\n", (utime + stime) / (double)sysconf(_SC_CLK_TCK));
    printf("Resident memory: %ld kB\n", vmrss);

    return 0;
}
