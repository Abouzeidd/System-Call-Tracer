#ifndef OUTPUT_H
#define OUTPUT_H

#include <sys/types.h>

typedef struct {
    const char *name;
    long        args[6];
    int         arg_count;
    int         valid;
} pending_syscall_t;

void output_on_entry(pending_syscall_t *pending,
                     const char *name,
                     long args[6],
                     int arg_count);

void output_on_exit(const pending_syscall_t *pending,
                    long retval,
                    pid_t child_pid);

#endif