#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "tracer.h"
#include "syscalls.h"
#include "output.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    pid_t child = fork();

    if (child == -1) {
        perror("fork failed");
        return 1;
    }

    if (child == 0) {
        /* --- CHILD PROCESS --- */
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("ptrace TRACEME failed");
            exit(1);
        }
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(1);

    } else {
        /* --- PARENT PROCESS (TRACER) --- */
        int status;
        int is_entry_stop = 1;
        pending_syscall_t pending = {0};

        /* Wait for child to stop at first instruction */
        if (waitpid(child, &status, 0) == -1) {
            perror("waitpid failed (initial stop)");
            return 1;
        }

        /* Distinguish syscall stops from other signals */
        if (ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD) == -1) {
            perror("ptrace SETOPTIONS failed");
            return 1;
        }

        while (WIFSTOPPED(status)) {
            if (ptrace(PTRACE_SYSCALL, child, NULL, NULL) == -1) {
                perror("ptrace SYSCALL failed");
                break;
            }

            if (waitpid(child, &status, 0) == -1) {
                perror("waitpid failed (loop)");
                break;
            }

            if (WIFEXITED(status)) break;

            if (is_entry_stop) {
                /* ── ENTRY: save syscall name + args ── */
                long id = get_syscall_id(child);
                if (id == -1) {
                    fprintf(stderr, "Warning: failed to get syscall id\n");
                    is_entry_stop = 0;
                    continue;
                }

                const char *name = get_syscall_name(id);
                long args[6];
                get_syscall_args(child, args);
                const syscall_meta *meta = get_syscall(id);
                int arg_count = (meta != NULL) ? meta->argc : 3;

                output_on_entry(&pending, name, args, arg_count);
                is_entry_stop = 0;

            } else {
                /* ── EXIT: get return value and print ── */
                long retval = get_syscall_return(child);
                output_on_exit(&pending, retval, child);
                pending.valid = 0;
                is_entry_stop = 1;
            }
        }

        printf("\n[DONE] Target process exited.\n");
    }
    return 0;
}
