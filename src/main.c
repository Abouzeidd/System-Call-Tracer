#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include "tracer.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    pid_t child = fork();

    if (child == 0) {
        // --- CHILD PROCESS ---
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(1);
    } else {
        // --- PARENT PROCESS (TRACER) ---
        int status;
        waitpid(child, &status, 0); // Wait for child to stop at exec

        // Tell child to continue until it hits a syscall
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);

        while (WIFSTOPPED(status)) {
            waitpid(child, &status, 0);

            // Here is where Member 3's code will eventually go!
            printf("[DEBUG] Syscall intercepted!\n");

            // Restart child and stop at next syscall entry/exit
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        }
        printf("Process exited.\n");
    }
    return 0;
}
