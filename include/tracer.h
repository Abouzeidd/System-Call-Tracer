#ifndef TRACER_H
#define TRACER_H

#include <sys/types.h>

// This ensures everyone uses the same function names
const char* get_syscall_name(long syscall_id);
void extract_registers(pid_t child_pid, long args[6]);

#endif
