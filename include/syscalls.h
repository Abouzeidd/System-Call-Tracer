#ifndef SYSCALLS_H
#define SYSCALLS_H

typedef struct {

    int id;
    const char *name;
    int argc;

} syscall_meta;

const syscall_meta* get_syscall(int id);

#endif

