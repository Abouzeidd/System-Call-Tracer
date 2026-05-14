🧪 Member 5: Integration & Git Manager
Name: Mariam Mazen
Responsibilities: Project Integration, Error Handling, Git Workflow, and Testing Suite.
1. Project Structure & Build System
Set up and maintained the Makefile used to compile all modules into the final binary.
makefileCC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/decoder.c src/registers.c src/syscalls.c src/formatter.c src/output.c
TARGET = strace_tracer
2. Error Handling
Added if (ptrace(...) == -1) checks across all ptrace calls in the project to ensure the tracer fails gracefully instead of silently producing wrong output.
FileCalls Protectedsrc/main.cfork, PTRACE_TRACEME, PTRACE_SETOPTIONS, PTRACE_SYSCALL, waitpidsrc/registers.cPTRACE_GETREGS (×3)src/output.cPTRACE_PEEKDATA
3. Testing Suite
Ran the tracer against 5 different programs and documented the results. Full output is available in docs/TEST_REPORT.md.
#ProgramStatusKey Output1ls✅ Passwrite(1, "Makefile   docs...", 48) = 482echo hello✅ Passwrite(1, "hello\n", 6) = 63cat /etc/hostname✅ Passwrite(1, "LAPTOP-EG9CRD6K\n", 16) = 164pwd✅ Passwrite(1, "/root/System-Call-Tracer-1\n", 27) = 275whoami✅ Passwrite(1, "root\n", 5) = 5