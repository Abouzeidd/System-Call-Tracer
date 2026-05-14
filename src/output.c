#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <errno.h>
#include "output.h"

static void read_string(pid_t pid, long addr, char *buf, int maxlen)
{
    int i = 0;
    while (i < maxlen - 1) {
        errno = 0;
        long word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (word == -1 && errno != 0) {
            perror("ptrace PEEKDATA failed (read_string)");
            break;
        }
        char *bytes = (char *)&word;
        for (int j = 0; j < (int)sizeof(long); j++) {
            if (i >= maxlen - 1) break;
            buf[i++] = bytes[j];
            if (bytes[j] == '\0') return;
        }
    }
    buf[i] = '\0';
}

static int arg_is_string(const char *name, int arg_index)
{
    static const char *path_syscalls[] = {
        "open", "openat", "stat", "lstat", "access",
        "execve", "unlink", "mkdir", "rmdir", "chdir",
        "rename", "chmod", "chown", "readlink", "creat",
        "truncate", NULL
    };
    if (arg_index == 0) {
        for (int i = 0; path_syscalls[i]; i++)
            if (strcmp(name, path_syscalls[i]) == 0) return 1;
    }
    if (arg_index == 1 && strcmp(name, "write") == 0) return 1;
    return 0;
}

void output_on_entry(pending_syscall_t *pending,
                     const char *name,
                     long args[6],
                     int arg_count)
{
    pending->name      = name;
    pending->arg_count = arg_count;
    pending->valid     = 1;
    memcpy(pending->args, args, sizeof(long) * 6);
}

void output_on_exit(const pending_syscall_t *pending,
                    long retval,
                    pid_t child_pid)
{
    if (!pending->valid) return;

    fprintf(stderr, "%s(", pending->name);

    for (int i = 0; i < pending->arg_count; i++) {
        if (i > 0) fprintf(stderr, ", ");

        if (arg_is_string(pending->name, i) && pending->args[i] != 0) {
            char buf[256] = {0};
            read_string(child_pid, pending->args[i], buf, sizeof(buf));
            fprintf(stderr, "\"%s\"", buf);
        } else {
            
            long v = pending->args[i];
            if (v >= 0 && v <= 65535)
                fprintf(stderr, "%ld", v);
            else
                fprintf(stderr, "0x%lx", v);
        }
    }

    fprintf(stderr, ") = ");

    if (retval < 0 && retval > -4096)
        fprintf(stderr, "-1 /* error %ld */\n", -retval);
    else
        fprintf(stderr, "%ld\n", retval);
}
