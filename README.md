# `ptree` System Call

`ptree` is a Linux system call that traverses the family tree of running processes in pre-order from root and returns their information.

### Definition

The function prototype of `ptree` is as follows:

```c
ssize_t sys_ptree(struct pinfo *buf, size_t len);
```

The `buf` parameter is a user-space buffer for `pinfo` structures to save information about traversed processes. `len` is the length of the buffer, i.e., the number of allocated `pinfo` entries in the buffer.

`struct pinfo` is a struct that holds information about a single process, and its definition is as follows:

```c
struct pinfo {
  int64_t       state;       /* current state of the process */
  pid_t         pid;         /* process id */
  int64_t       uid;         /* user id of the process owner */
  char          comm[64];    /* name of the program executed */
  unsigned int  depth;       /* depth of the process in the process tree */
};
```

### Return value

If there was no error, `ptree` returns the number of `struct pinfo`s that were output to `buf`. If an error occured during system call execution, a negative error code is returned. There may be two cases:

- `-EINVAL` :  `buf` is `NULL`, or `len` is 0.
- `-EFAULT` : The user-space buffer `buf` is not accessible.

### Using `ptree` in user-space

The syscall number is registered as `294` in the kernel code, so you can call it using the `syscall()` C function.

Example:

```c
#include <sys/syscall.h>

syscall(294, buf, buf_len);
```

### Running the `ptree` test code

A test code for `ptree` is provided in `test/test_ptree.c`. The test code takes the buffer size as a command line argument and calls `ptree`  and pretty-prints the output buffer in a tree format with indentation.

A Makefile is provided in the same folder with the test code, so you can compile it like this:
```bash
cd test
make
```

The default compiler is configured as`aarch64-linux-gnu-gcc` in the Makefile, so make sure that the compiler is properly installed. Also, make sure to run the compiled executable in the appropriate architecture. We have tested only in Tizen OS.

Example:

```bash
root:~> ./test_ptree 15
swapper/0, 0, 0, 0
        systemd, 1, 1, 0
                dbus-daemon, 159, 1, 81
                systemd-journal, 163, 1, 0
                systemd-udevd, 175, 1, 0
                        systemd-udevd, 181, 1, 0
                        systemd-udevd, 182, 1, 0
                        systemd-udevd, 183, 1, 0
                        systemd-udevd, 184, 1, 0
                        systemd-udevd, 185, 1, 0
                        systemd-udevd, 186, 1, 0
                        systemd-udevd, 187, 1, 0
                        systemd-udevd, 188, 1, 0
                        systemd-udevd, 189, 1, 0
                        systemd-udevd, 190, 1, 0
```
