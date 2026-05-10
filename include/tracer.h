#ifndef TRACER_H
#define TRACER_H
#include <sys/types.h>

long get_syscall_id(pid_t child_pid);
const char* get_syscall_name(long syscall_id);

#endif
