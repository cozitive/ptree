#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "test_ptree.h"
#define SYS_PTREE 294

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "command: Invalid argument\n");
        return 1;
    }

    char *arg = argv[1];
    int i;
    for (i = 0; i < strlen(arg); i++) {
        if (!isdigit(arg[i])) {
            fprintf(stderr, "command: Invalid argument\n");
            return 1;
        }
    }
    int buf_len = atoi(arg);

    struct pinfo *buf = malloc(buf_len * sizeof(struct pinfo));
    int ret_len = syscall(SYS_PTREE, buf, buf_len);
    if (ret_len < 0) {
        perror("syscall");
        return 1;
    }
    for (int i = 0; i < ret_len; i++) {
        struct pinfo p = buf[i];
        for(int j = 0; j<p.depth; j++){
            printf("    ");
        }
        printf("[%d] %s, %d, %ld, %ld\n", i, p.comm, p.pid, p.state, p.uid);
    }
    return 0;
}