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

📂 Project StructurePlaintext.
├── include/
│   └── tracer.h          # Global definitions & structs
├── src/
│   ├── main.c            # Tracer loop & process management
│   ├── decoder.c         # Syscall mapping logic
│   └── registers.c       # PTRACE_GETREGS implementation
├── tests/
│   └── hello.c           # Sample program for validation
└── Makefile              # Build automation
🛠️ Implementation DetailsFeatureDescriptionPTRACE_SYSCALLEfficiently stops the child only at syscall boundaries.Dual-Stop LogicUses a state flag to track the Entry/Exit cycle of each call.x86-64 SupportTargeted extraction of the 6-register syscall calling convention.

---

## 🎯 Member 1: Core Engine & Syscall Decoder

**Name:** Abdelrahman Abouzeid (Team Lead)

**Responsibilities:** ptrace Framework, Process Lifecycle, and Syscall Mapping.

### 1. The ptrace Foundation

I designed and implemented the core event loop using the Linux `ptrace` API. This engine handles the critical "Dual-Stop" logic required to capture a full system call lifecycle.

* **Process Synchronization:** Managed the `fork()` / `exec()` handshake, ensuring the child process is stopped immediately before execution using `PTRACE_TRACEME`.
* **Dual-Stop Logic:** Implemented a state-machine in the main loop to distinguish between **Syscall Entry** (where we capture arguments) and **Syscall Exit** (where we capture the return value).
* **Performance:** Utilized `PTRACE_SYSCALL` to ensure the tracer only wakes up for kernel boundaries, significantly reducing CPU overhead compared to instruction-level single-stepping.

### 2. Syscall Decoder Subsystem

I developed the translation layer that converts raw kernel data into human-readable information.

* **Static Lookup Table:** Built a high-efficiency syscall table providing **$O(1)$ lookup time**. This translates raw syscall IDs (e.g., `1`) into names (e.g., `write`).
* **Metadata Mapping:** Assigned argument counts to each syscall, allowing the output module to know exactly how many registers to read for a clean display.

### 3. Architecture & Integration

As Team Lead, I defined the **Global Header (`tracer.h`)**, which served as the project's technical contract. This ensured:

* The **Register Module** knew exactly which `user_regs_struct` to populate.
* The **Output Module** had a consistent `pending_syscall` struct to format and print.

---

### Key Code Contributions:

| Function | Description |
| --- | --- |
| `main_trace_loop()` | The heartbeat of the program; manages signal handling and process states. |
| `get_syscall_name()` | Primary interface for the decoding logic. |
| `is_entry_stop` flag | Critical logic gate used to sync entry/exit phases. |

---

### Implementation Spotlight: The Dual-Stop Cycle

```c
// My logic ensures the tracer captures both sides of the kernel gate:
if (is_entry_stop) {
    // Capture RAX (ID) and RDI, RSI, RDX (Args)
    is_entry_stop = 0; 
} else {
    // Capture RAX (Return Value)
    is_entry_stop = 1;
}

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
