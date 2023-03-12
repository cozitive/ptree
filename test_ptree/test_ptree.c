#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include "test_ptree.h"
#define SYS_PTREE 294

int main() {
    struct pinfo *buf;
    int ret = syscall(SYS_PTREE, buf, 0);
    if (ret < 0) {
        perror("syscall");
        return 1;
    }
    printf("syscall returned %d\n", ret);
    return 0;
}