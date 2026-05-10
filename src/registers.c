#include <sys/ptrace.h>
#include <sys/user.h>
#include <stddef.h>
#include "tracer.h"

long get_syscall_id(pid_t child_pid) {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
    return regs.orig_rax; // This is the ID on 64-bit Linux
}
