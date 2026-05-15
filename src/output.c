#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <errno.h>
#include "output.h"
#include "syscalls.h"

static void read_string(pid_t pid, long addr, char *buf, int maxlen)
{
    int i = 0;
    if (addr == 0) {
        buf[0] = '\0';
        return;
    }
    
    while (i < maxlen - 1) {
        errno = 0;
        long word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (word == -1 && errno != 0) {
            // Silent failure - address may be invalid or protected
            buf[i] = '\0';
            return;
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
    // First argument strings (filenames, paths)
    static const char *path_syscalls_arg0[] = {
        "open", "openat", "stat", "lstat", "access", "chmod", "chown",
        "lchown", "link", "unlink", "symlink", "readlink", "mkdir", "rmdir",
        "creat", "truncate", "rename", "getxattr", "lgetxattr", "setxattr",
        "lsetxattr", "removexattr", "lremovexattr", "mkdirat", "mknodat",
        "unlinkat", "renameat", "linkat", "symlinkat", "readlinkat",
        "fchmodat", "faccessat", "statx", "listxattr", "llistxattr",
        "getcwd", "chdir", "mount", "umount2", "sethostname", "setdomainname",
        NULL
    };

    // Second argument strings
    static const char *path_syscalls_arg1[] = {
        "link", "symlink", "rename", "renameat",
        NULL
    };

    // Check first argument strings
    if (arg_index == 0) {
        for (int i = 0; path_syscalls_arg0[i] != NULL; i++) {
            if (strcmp(name, path_syscalls_arg0[i]) == 0) {
                return 1;
            }
        }
    }

    // Check second argument strings
    if (arg_index == 1) {
        for (int i = 0; path_syscalls_arg1[i] != NULL; i++) {
            if (strcmp(name, path_syscalls_arg1[i]) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

void output_on_entry(pending_syscall_t *pending,
                     const char *name,
                     long args[6],
                     int arg_count)
{
    // Store the syscall info in pending
    pending->name = name;
    pending->arg_count = arg_count;
    for (int i = 0; i < arg_count && i < 6; i++) {
        pending->args[i] = args[i];
    }
    pending->valid = 1;
}

void output_on_exit(const pending_syscall_t *pending,
                    long retval,
                    pid_t child_pid)
{
    if (!pending || !pending->valid) {
        return;
    }

    fprintf(stderr, "%s(", pending->name);
    
    for (int i = 0; i < pending->arg_count; i++) {
        if (i > 0) fprintf(stderr, ", ");
        
        if (arg_is_string(pending->name, i) && pending->args[i] != 0) {
            char buf[256] = {0};
            read_string(child_pid, pending->args[i], buf, sizeof(buf));
            
            // Only print if we got a valid string
            if (buf[0] != '\0') {
                fprintf(stderr, "\"%s\"", buf);
            } else {
                fprintf(stderr, "0x%lx", pending->args[i]);
            }
        } else {
            long v = pending->args[i];
            // Print small positive numbers as decimal, others as hex
            if (v >= 0 && v <= 65535) {
                fprintf(stderr, "%ld", v);
            } else {
                fprintf(stderr, "0x%lx", v);
            }
        }
    }
    
    fprintf(stderr, ") = ");
    
    // Format return value
    if (retval < 0 && retval > -256) {
        fprintf(stderr, "-1 /* error %ld */", -retval);
    } else if (retval >= 0 && retval <= 65535) {
        fprintf(stderr, "%ld", retval);
    } else {
        fprintf(stderr, "0x%lx", retval);
    }
    
    fprintf(stderr, "\n");
}
