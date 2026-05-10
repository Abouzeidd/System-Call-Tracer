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
        /* --- CHILD PROCESS --- */
        // Allow the parent to trace this process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        
        // Execute the target program
        execvp(argv[1], &argv[1]);
        
        // If execvp returns, an error occurred
        perror("execvp");
        exit(1);
    } else {
        /* --- PARENT PROCESS (TRACER) --- */
        int status;
        int is_entry_stop = 1; // Toggle to handle entry/exit stops

        // Wait for the child to stop at the first instruction (execve)
        waitpid(child, &status, 0);

        // Set options to distinguish syscall stops from other signals
        ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD);

        // Main tracing loop
        while (WIFSTOPPED(status)) {
            // Tell the child to run until the next syscall event
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
            waitpid(child, &status, 0);

            if (WIFEXITED(status)) break;

            /* Only process on 'Entry'. This prevents the double-printing 
               you saw in your previous terminal test.
            */
            if (is_entry_stop) {
                // 1. Get the Syscall ID (Member 3's logic)
                long id = get_syscall_id(child);

                // 2. Translate ID to Name (Member 2's logic)
                const char* name = get_syscall_name(id);

                // 3. Output the result
                printf("[TRACER] Syscall: %-15s (ID: %ld)\n", name, id);

                is_entry_stop = 0; // Next stop will be the 'Exit' stop
            } else {
                is_entry_stop = 1; // Next stop will be a new 'Entry' stop
            }
        }
        printf("\n[DONE] Target process exited.\n");
    }

    return 0;
}
