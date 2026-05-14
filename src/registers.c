#include <sys/ptrace.h>
#include <sys/user.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include "tracer.h"


long get_syscall_id(pid_t child_pid) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1) {
        perror("ptrace GETREGS failed (get_syscall_id)");
        return -1;
    }
    return regs.orig_rax;
}


void get_syscall_args(pid_t child_pid, long args[6]) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1) {
        perror("ptrace GETREGS failed (get_syscall_args)");
        for (int i = 0; i < 6; i++) args[i] = 0;
        return;
    }

    args[0] = (long)regs.rdi;
    args[1] = (long)regs.rsi;
    args[2] = (long)regs.rdx;
    args[3] = (long)regs.r10;
    args[4] = (long)regs.r8;
    args[5] = (long)regs.r9;
}


long get_syscall_return(pid_t child_pid) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1) {
        perror("ptrace GETREGS failed (get_syscall_return)");
        return -1;
    }
    return (long)regs.rax;
}
