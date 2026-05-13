# Member 4 — Output Formatter Documentation

**Name:** Kaazzy  
**Role:** Logic & Output Formatter  
**Files:** `src/output.c`, `include/output.h`  
**Also modified:** `src/main.c`, `Makefile`

---

## Problem Statement

The tracer intercepts system calls but raw data is meaningless without
formatting. When a process calls `open("/etc/passwd", O_RDONLY)`, ptrace
gives us numbers — syscall ID 2, argument 0 is a memory address, argument
1 is an integer flag. The output module's job is to turn those numbers into:

```
open("/etc/passwd", 0) = 3
```

This matches the output of the real Linux `strace` tool.

---

## The Core Design Challenge — Entry vs Exit

`ptrace` with `PTRACE_SYSCALL` stops the child process **twice** per syscall:

```
─────────────────────────────────────────────────────
  ENTRY stop          kernel runs syscall        EXIT stop
      │                       │                      │
  args available         (we wait)            return value
  name available                               available
  return = garbage                             args = same
─────────────────────────────────────────────────────
```

This means we **cannot** print a complete line at either stop alone:
- At entry: we have the name and args but no return value yet
- At exit: we have the return value but registers no longer hold the original args

**Solution:** The `pending_syscall_t` struct saves entry data so it is
available when the exit stop fires.

---

## Data Flow

```
Child process executes a syscall
         │
         ▼
[ENTRY STOP] ptrace signals parent
         │
         ├── get_syscall_id(child)        → long id
         ├── get_syscall_name(id)         → const char *name
         ├── get_syscall_args(child, args) → long args[6]
         ├── get_syscall(id)->argc        → int arg_count
         │
         ▼
output_on_entry(&pending, name, args, arg_count)
         │  saves everything into pending_syscall_t
         │
[child continues executing]
         │
[EXIT STOP] ptrace signals parent
         │
         ├── get_syscall_return(child)    → long retval
         │
         ▼
output_on_exit(&pending, retval, child_pid)
         │
         ├── reads string args from child memory (PTRACE_PEEKDATA)
         ├── formats: syscall_name(arg1, arg2, ...) = retval
         │
         ▼
fprintf(stderr, "write(1, \"hello\", 5) = 5\n")
```

---

## Component Documentation

### 1. `pending_syscall_t` — `include/output.h`

```c
typedef struct {
    const char *name;    /* syscall name e.g. "write"       */
    long        args[6]; /* raw argument values              */
    int         arg_count; /* number of args to print        */
    int         valid;   /* 1 = entry data saved, 0 = empty */
} pending_syscall_t;
```

**Design decision:** `args` is copied by value using `memcpy` rather than
storing a pointer. This is essential because `args` in `main.c` is a local
stack variable that gets overwritten on the next iteration of the tracing
loop. Without `memcpy`, we would print garbage values.

---

### 2. `output_on_entry()` — `src/output.c`

```c
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
```

Called by `main.c` on every entry stop. Sets `valid = 1` so the exit
handler knows real data is waiting.

---

### 3. `output_on_exit()` — `src/output.c`

```c
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
    print_return_value(retval);
}
```

**Why `stderr`?** The traced program's own output goes to `stdout`. Using
`stderr` keeps the trace separate — if you run `./strace_tracer ls > out.txt`,
the `ls` output goes to the file but the syscall trace still appears on screen.
This is identical to how real `strace` behaves.

---

### 4. `arg_is_string()` — `src/output.c`

**Design rationale:** The x86-64 ABI passes all syscall arguments as raw
64-bit integers. There are no runtime type tags — a pointer and an integer
are indistinguishable at the binary level. This function encodes knowledge
of the Linux syscall ABI from the man pages to identify which arguments
are string pointers for known syscalls.

```c
static int arg_is_string(const char *name, int arg_index)
{
    static const char *path_syscalls[] = {
        "open", "openat", "stat", "lstat", "access",
        "execve", "unlink", "mkdir", "rmdir", "chdir",
        "rename", "chmod", "chown", "readlink", "creat",
        "truncate", NULL   /* NULL sentinel ends the list */
    };
    if (arg_index == 0) {
        for (int i = 0; path_syscalls[i]; i++)
            if (strcmp(name, path_syscalls[i]) == 0) return 1;
    }
    if (arg_index == 1 && strcmp(name, "write") == 0) return 1;
    return 0;
}
```

**Why a static table over a hashmap?** A static array is cache-friendly,
requires zero memory allocation, and the lookup cost is negligible for
~20 entries. The syscall names are known at compile time so a dynamic
structure would add complexity with no benefit.

---

### 5. `read_string()` — Memory Safety

```c
static void read_string(pid_t pid, long addr,
                        char *buf, int maxlen)
{
    int i = 0;
    while (i < maxlen - 1) {
        long word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (word == -1 && errno != 0) break;
        char *bytes = (char *)&word;
        for (int j = 0; j < (int)sizeof(long); j++) {
            if (i >= maxlen - 1) break;
            buf[i++] = bytes[j];
            if (bytes[j] == '\0') return;
        }
    }
    buf[i] = '\0';
}
```

**Why `PTRACE_PEEKDATA` instead of direct pointer dereference?**
The child process has its own virtual address space. Dereferencing a
pointer from the child directly in the parent would cause a segfault or
read wrong memory. `PTRACE_PEEKDATA` asks the kernel to perform the read
on our behalf, safely crossing the process boundary.

**Memory safety measures:**
- `maxlen - 1` bound prevents buffer overflow — always leaves room for `\0`
- `errno` check distinguishes a valid `-1` return from a real error
- `buf[i] = '\0'` always null-terminates even if the loop exits early
- `buf[256] = {0}` zero-initializes so partial reads are still safe strings

**Why `PTRACE_PEEKDATA` over `process_vm_readv`?**
`PTRACE_PEEKDATA` works within the existing ptrace session already
established by the tracer. `process_vm_readv` is faster for large reads
but requires additional permissions setup. For short strings (filenames,
paths), `PTRACE_PEEKDATA` is simpler and sufficient.

---

### 6. `print_return_value()` — Error Code Handling

```c
void print_return_value(long retval)
{
    if (retval < 0 && retval > -4096)
        fprintf(stderr, "-1 /* error %ld */\n", -retval);
    else
        fprintf(stderr, "%ld\n", retval);
}
```

**The -4096 threshold** comes from the Linux kernel convention. The kernel
reserves the range `[-4096, -1]` exclusively for errno error codes. Any
syscall return in that range is guaranteed to be an error — never a valid
file descriptor, byte count, or address. This is the same check that glibc
uses internally when converting raw syscall returns into `errno` values.

---

## Testing

```bash
./strace_tracer ls
./strace_tracer echo hello
./strace_tracer cat /etc/hostname
./strace_tracer pwd
./strace_tracer whoami
```

**Sample output:**
```
openat("", 0x7c3cdebd48b0, 0x80000, 0) = 3
fstat(3, 0x7c3cdec04800) = 0
close(3) = 0
access("/etc/selinux/config", 0) = -1 /* error 2 */
write(1, "hello  include  Makefile...", 77) = 77
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## Summary of Changes

| File | Change |
|---|---|
| `include/output.h` | New file — defines `pending_syscall_t` and function signatures |
| `src/output.c` | New file — full output formatter implementation |
| `src/main.c` | Modified — integrated `output_on_entry` and `output_on_exit` into tracer loop |
| `Makefile` | Modified — added `src/output.c` to `SRC` list |
| `src/formatter.c` | Modified — includes `output.h` |