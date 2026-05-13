# System Call Tracer (strace-clone)
A lightweight Linux utility designed to intercept and record system calls made by a process. Developed as a group project for the Operating Systems course.

## 👥 The Team
* **Abdelrahman Abouzeid** - Team Lead & Core Engine Logic
* **Malak Amir** - Syscall Decoder & Name Mapping
* **Alaa Tamer** - Register Extraction & CPU Logic
* **Mohamed Osama** - Error Handling & Edge Cases
* **Mariam Mazen** - Documentation & Testing Suite

---

## 🚀 Quick Start

### Prerequisites
* Linux Environment (Ubuntu/WSL2 recommended)
* GCC Compiler
* Make build tool

### Installation & Build
1. Clone the repository:
```bash
git clone https://github.com/Abouzeidd/System-Call-Tracer.git
cd System-Call-Tracer
```

2. Build:
```bash
make
```

3. Run:
```bash
./strace_tracer ls
./strace_tracer echo hello
./strace_tracer cat /etc/hostname
```

---

## x86-64 ABI

This section explains how the Linux kernel receives
system call requests from user-space programs on x86-64.

Linux x86-64 system calls follow a fixed ABI convention.

The syscall number is stored in the RAX register.

Arguments are passed using:
- RDI → first argument
- RSI → second argument
- RDX → third argument
- R10 → fourth argument
- R8  → fifth argument
- R9  → sixth argument

### Example: write(1, "hello", 5)

| Register | Value | Meaning            |
|----------|-------|--------------------|
| RAX      | 1     | syscall number     |
| RDI      | 1     | file descriptor    |
| RSI      | 0x... | pointer to "hello" |
| RDX      | 5     | number of bytes    |

After execution, RAX holds the return value (bytes written, or negative error code).

---

## Static Syscall Table Design

A static syscall table was used to provide fast lookup performance and simplify syscall decoding.

Each syscall entry stores:
- syscall number
- syscall name
- argument count

This design avoids runtime parsing and reduces overhead during tracing.

### Why not a HashMap?

A static array was chosen over a hashmap because:
- Syscall numbers are known at compile time
- The table is small (~300 entries max)
- No memory allocation needed at runtime
- Cache-friendly sequential access

---

## Register Extraction & CPU Logic

All register access is handled in `src/registers.c` using the x86-64 ABI convention.

Three functions were implemented to extract data directly from the CPU registers of the traced process:

| Function | Register | Description |
|---|---|---|
| `get_syscall_id()` | `orig_rax` | Returns the syscall number at entry |
| `get_syscall_args()` | `rdi, rsi, rdx, r10, r8, r9` | Returns all 6 arguments passed to the syscall |
| `get_syscall_return()` | `rax` | Returns the return value after syscall exits |

### Why orig_rax and not rax?

The Linux kernel saves the original syscall number in `orig_rax` before execution.
After the syscall runs, `rax` is overwritten with the return value.
Using `orig_rax` guarantees we always read the correct syscall number at entry.

---

## 🎨 Output Formatter — Member 4 (Kaazzy)

The output module formats all tracer data into human-readable lines
that match the style of real `strace`.

**Files:** `src/output.c`, `include/output.h`  
**Full documentation:** [docs/MEMBER4-output-formatter.md](docs/MEMBER4-output-formatter.md)

### Data Flow

```
Child Process                   Parent Tracer
─────────────                   ─────────────────────────────────────
execvp(target)
    │
    │  SIGTRAP (syscall entry)
    ├──────────────────────────► get_syscall_id()      [Member 3]
    │                            get_syscall_name()    [Member 2]
    │                            get_syscall_args()    [Member 3]
    │                                 │
    │                            output_on_entry()     [Member 4]
    │                            saves: name, args, arg_count
    │                                 │
    │  SIGTRAP (syscall exit)         │
    ├──────────────────────────► get_syscall_return()  [Member 3]
    │                                 │
    │                            output_on_exit()      [Member 4]
    │                            reads child memory via PTRACE_PEEKDATA
    │                                 │
    │                                 ▼
    │                            write(1, "hello", 5) = 5
```

### Component Breakdown

| Component | File | Purpose |
|---|---|---|
| `pending_syscall_t` | `include/output.h` | Struct that bridges entry and exit stops |
| `output_on_entry()` | `src/output.c` | Saves syscall name and args at entry stop |
| `output_on_exit()` | `src/output.c` | Prints complete formatted line at exit stop |
| `arg_is_string()` | `src/output.c` | Detects which arguments are string pointers |
| `read_string()` | `src/output.c` | Reads strings from child memory via ptrace |
| `print_return_value()` | `src/output.c` | Formats return values and errno error codes |

### Sample Output

```
openat("", 0x7c3cdebd48b0, 0x80000, 0) = 3
fstat(3, 0x7ffd3a694070) = 0
close(3) = 0
access("/etc/selinux/config", 0) = -1 /* error 2 */
write(1, "hello  include  Makefile...", 77) = 77
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```