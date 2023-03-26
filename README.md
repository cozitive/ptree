# `ptree` System Call

`ptree` is a Linux system call that traverses the family tree of running processes in pre-order from root and returns their information.

## Definition

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

## Implementation
To implement the pre-order traverse, `ptree` used a stack instead of recursive function call. The built-in linked list in kernel is used as a stack.
```c
static LIST_HEAD(stack);
```

A Stack element contains each task information and a pointer to list head:
```c
struct stack_element {
    struct task_struct *task;
    struct list_head head;
    unsigned int depth;
};
```

After handling `EINVAL` and `EFAULT` error, `ptree` allocates memory space for storing `pinfo`s and start to investigate all processes. 
```c
pinfos = kmalloc(len * sizeof(struct pinfo), GFP_KERNEL);
```

Before the traverse, `tasklist_lock` is locked to preserve the process list.
```c
read_lock(&tasklist_lock);
```

The first element pushed to the stack represents the `init`  process.
```c
init = kmalloc(sizeof(struct stack_element), GFP_ATOMIC);
init->depth = 0;
init->task = &init_task;
INIT_LIST_HEAD(&init->head);
list_add(&init->head, &stack);
```

After pushing `init`, `pinfo` dives into the traverse loop:
1. task information at the top of the stack is stored in `pinfos`.
```c
pinfos[index].state = top_task->state;
pinfos[index].pid = top_task->pid;
pinfos[index].uid = top_task->cred->uid.val;
pinfos[index].depth = top_element->depth;
strncpy(pinfos[index].comm, top_task->comm, TASK_COMM_LEN);
```

2. all children of the top task is pushed into the stack, in reverse order by PID.
```c
list_for_each_prev(child_head, &top_task->children) {
    child_task = list_entry(child_head, struct task_struct, sibling);
    child_element = kmalloc(sizeof(struct stack_element), GFP_ATOMIC);
    child_element->task = child_task;
    child_element->depth = top_element->depth + 1;
    INIT_LIST_HEAD(&child_element->head);
    list_add(&child_element->head, &stack);
}
```

3. the top stack element is popped.
```c
list_del(top_head);
```

`ptree` syscall iterates these operations while the stack become empty, or the result buffer become full.

After the termination of the loop, `tasklist_lock` is unlocked.
```c
read_unlock(&tasklist_lock);
```

If the loop is terminated due to insufficient buffer size, the stack may be not empty. In this case, the stack should be emptied, and all stack elements should be freed.
```c
while (!list_empty(&stack)) {
    struct list_head *node = stack.next;
    struct stack_element *element = list_entry(node, struct stack_element, head);
    list_del(node);
    if (element != NULL) {
        kfree(element);
    }
}
```

Lastly, `pinfos` should be copied to a user space buffer `buf`, and be freed.
```c
copy_to_user(buf, pinfos, index * sizeof(struct pinfo));
kfree(pinfos);
```

### Preventing deadlock during the lock
While holding `tasklist_lock`, letting the kernel thread sleep causes a deadlock. To prevent it, `kmalloc()` is called with `GFP_ATOMIC` flag.
```c
child_element = kmalloc(sizeof(struct stack_element), GFP_ATOMIC);
```

To call `kfree()` after releasing the lock, pointers to the popped stack elements are stored in a list called `garbage`, and be freed at once.
```c
static LIST_HEAD(garbage);

read_lock(&tasklist_lock);

while (!list_empty(&stack)) {
    ...
    list_del(top_head);
    list_add(top_head, &garbage);
}

read_unlock(&tasklist_lock);

while (!list_empty(&garbage)) {
    struct list_head *node = garbage.next;
    struct stack_element *element = list_entry(node, struct stack_element, head);
    list_del(node);
    if (element != NULL) {
        kfree(element);
    }
}
```

## Using `ptree` in user space

The syscall number is registered as `294` in the kernel code, so you can call it using the `syscall()` C function.

Example:

```c
#include <sys/syscall.h>

syscall(294, buf, buf_len);
```

## Running the test code

A test code for `ptree` is provided in `test/test_ptree.c`. The test code takes the buffer size as a command line argument and calls `ptree`  and pretty-prints the output buffer in a tree format with indentation.

A Makefile is provided in the same folder with the test code, so you can compile it like this:
```bash
$ cd test
$ make
```

The default compiler is configured as`aarch64-linux-gnu-gcc` in the Makefile, so make sure that the compiler is properly installed. Also, make sure to run the compiled executable in the appropriate architecture. We have tested only in Tizen OS.

To run the compiled binary, you should put buffer size as the second command-line argument(`argv[1]`).

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
