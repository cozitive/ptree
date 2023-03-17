#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "test_ptree.h"
#include <errno.h>
#define SYS_PTREE 294

int main(int argc, char *argv[]) {
    // Validate whether a single command-line argument is a integer.
    if (argc != 2) {
        fprintf(stderr, "Error: Invalid argument input\n");
        return 1;
    }
    char *arg = argv[1];
    for (int i = 0; i < strlen(arg); i++) {
        if (!isdigit(arg[i])) {
            fprintf(stderr, "Error: Invalid argument input\n");
            return 1;
        }
    }

    // Invoke ptree syscall.
    int buf_len = atoi(arg);
    struct pinfo *buf = malloc(buf_len * sizeof(struct pinfo));
    int ret_len = syscall(SYS_PTREE, buf, buf_len);

    // Handle error in ptree syscall.
    if (ret_len < 0) {
        if (errno == EINVAL)
            printf("Error: ptree() returned -EINVAL\n");
        else if (errno == EFAULT)
            printf("Error: ptree() returned -EFAULT\n");
        else
            printf("Error: %s\n", strerror(errno));
        free(buf);
        return 1;
    }

    // Print the result of ptree syscall.
    for (int i = 0; i < ret_len; i++) {
        struct pinfo p = buf[i];
        for(int j = 0; j < p.depth; j++){
            printf("\t");
        }
        printf("%s, %d, %ld, %ld\n", p.comm, p.pid, p.state, p.uid);
    }

    // Free allocated memory.
    free(buf);
    return 0;
}
