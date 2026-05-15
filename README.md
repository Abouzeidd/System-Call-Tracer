## 👥 Team

| Name | Role | Responsibilities |
|------|------|------------------|
| **Abdelrahman Abouzeid** | Team Lead & Core Engine | ptrace Framework, Process Lifecycle, Syscall Mapping |
| **Malak Amir** | Syscall Decoder | Syscall Name Mapping & Metadata |
| **Alaa Tamer** | Register Logic | Register Extraction & CPU Operations |
| **Mohamed Osama** | Error Handling | Edge Cases & Resilience |
| **Mariam Mazen** | Integration & Testing | Build System, Testing Suite, Documentation |

---

## 🚀 Quick Start

### Prerequisites

```bash
# Linux Environment (Ubuntu/WSL2 recommended)
sudo apt-get update
sudo apt-get install build-essential
```

- **Linux Environment** (Ubuntu/WSL2 recommended)
- **GCC Compiler**
- **Make** build tool

### Installation & Build

```bash
# Clone the repository
git clone https://github.com/Abouzeidd/System-Call-Tracer.git
cd System-Call-Tracer

# Build
make

# Run examples
./strace_tracer ls
./strace_tracer echo "hello"
./strace_tracer cat /etc/hostname
```

### Sample Output

```
write(1, "hello include Makefile...", 77) = 77
open("/etc/selinux/config", 0) = -1 /* error 2 */
fstat(3, 0x7ffd3a694070) = 0
close(1) = 0
[DONE] Target process exited.
```

---

## 📂 Project Structure

```
System-Call-Tracer/
├── include/
│   ├── tracer.h          # Global definitions & structs
│   └── output.h          # Output formatter interface
├── src/
│   ├── main.c            # Tracer loop & process management
│   ├── decoder.c         # Syscall mapping logic
│   ├── registers.c       # Register extraction (x86-64)
│   ├── output.c          # Output formatting
│   └── syscalls.c        # Static syscall table
├── tests/
│   └── hello.c           # Sample program for validation
├── docs/
│   ├── MEMBER1-core-engine.md
│   ├── MEMBER2-decoder.md
│   ├── MEMBER3-registers.md
│   ├── MEMBER4-output-formatter.md
│   ├── MEMBER5-testing.md
│   └── TEST_REPORT.md
└── Makefile              # Build automation
```

---

## 🛠️ Technical Details

### Core Features

| Feature | Implementation | Benefit |
|---------|-----------------|---------|
| **PTRACE_SYSCALL** | Efficient kernel boundary tracking | Only interrupts on syscall entry/exit, not every instruction |
| **Dual-Stop Logic** | State machine for entry/exit cycle | Captures both arguments and return values |
| **x86-64 ABI Support** | Register-based argument extraction | Correctly reads 6-argument calling convention |
| **Static Lookup Table** | O(1) syscall number → name mapping | Fast, cache-friendly performance |
| **Memory Reading** | PTRACE_PEEKDATA for child process memory | Enables string argument extraction |

### Architecture Overview

```
┌─────────────────┐
│  Child Process  │
│   execvp(bin)   │
└────────┬────────┘
         │ SIGTRAP (entry)
         ▼
   ┌──────────────────────┐
   │   Tracer Parent      │
   │  • get_syscall_id()  │
   │  • get_syscall_args()│
   │  • format output     │
   └──────────────────────┘
         │ SIGTRAP (exit)
         ▼
    Print Result
```

---

## 📋 How It Works

### The Dual-Stop Cycle

The tracer operates by catching the same system call **twice**:

1. **Entry Stop** → Read syscall number (RAX) and arguments (RDI, RSI, RDX, R10, R8, R9)
2. **Exit Stop** → Read return value (RAX)

```c
if (is_entry_stop) {
    // Capture syscall ID and arguments
    syscall_id = get_syscall_id();        // from orig_rax
    args = get_syscall_args();            // from RDI, RSI, RDX, R10, R8, R9
    is_entry_stop = 0;
} else {
    // Capture return value
    return_value = get_syscall_return();  // from rax
    is_entry_stop = 1;
}
```

### x86-64 System Call ABI

All syscalls on x86-64 Linux follow a fixed calling convention:

| Register | Purpose | Example |
|----------|---------|---------|
| **RAX** | Syscall number (entry) / Return value (exit) | 1 = write() |
| **RDI** | 1st argument | file descriptor |
| **RSI** | 2nd argument | buffer pointer |
| **RDX** | 3rd argument | byte count |
| **R10** | 4th argument | flags |
| **R8** | 5th argument | - |
| **R9** | 6th argument | - |

#### Example: `write(1, "hello", 5)`

| Register | Value | Meaning |
|----------|-------|---------|
| RAX | 1 | syscall: write |
| RDI | 1 | fd: stdout |
| RSI | 0x... | buffer address |
| RDX | 5 | bytes to write |

→ **Result in RAX**: 5 (bytes written)

### Why `orig_rax` and not `rax`?

The Linux kernel preserves the original syscall number in **`orig_rax`** because:
- After execution, `rax` is overwritten with the return value
- Using `orig_rax` guarantees correct syscall ID at entry

---

## Member Responsibilities

### 🎯 Member 1: Core Engine Logic (Abdelrahman)

**Focus:** ptrace Framework & Process Lifecycle

- **Process Synchronization**: fork() / exec() handshake with PTRACE_TRACEME
- **Dual-Stop State Machine**: Distinguishes entry vs. exit stops
- **Performance Optimization**: PTRACE_SYSCALL instead of single-stepping
- **Global Contract**: Defined tracer.h as the technical specification

**Key Functions:**
- `main_trace_loop()` — Main tracer heartbeat
- `get_syscall_name()` — Syscall ID → name translation
- `is_entry_stop` flag — Entry/exit synchronization

---

### 📍 Member 2: Syscall Decoder (Malak)

**Focus:** Human-Readable Syscall Translation

- **Static Lookup Table**: O(1) syscall number → name mapping
- **Metadata Mapping**: Argument count per syscall
- **Integration**: Provides consistent data to output module

**Design Choice:** Static array vs. hashmap
- ✅ Syscall numbers known at compile-time
- ✅ Small table (~300 entries)
- ✅ Zero runtime allocation
- ✅ Cache-friendly sequential access

---

### 🔧 Member 3: Register Extraction (Alaa)

**Focus:** CPU Register Access & x86-64 ABI

**Three Core Functions:**

| Function | Register | Returns |
|----------|----------|---------|
| `get_syscall_id()` | `orig_rax` | Syscall number |
| `get_syscall_args()` | `rdi, rsi, rdx, r10, r8, r9` | All 6 arguments |
| `get_syscall_return()` | `rax` | Return value / error code |

**Implementation:** Uses PTRACE_GETREGS to extract user_regs_struct

---

### 🎨 Member 4: Output Formatter (TBD)

**Focus:** Human-Readable Output Formatting

**Key Components:**

| Component | File | Purpose |
|-----------|------|---------|
| `pending_syscall_t` struct | output.h | Bridges entry & exit stops |
| `output_on_entry()` | output.c | Saves syscall name & args |
| `output_on_exit()` | output.c | Prints formatted line |
| `arg_is_string()` | output.c | Detects string arguments |
| `read_string()` | output.c | Reads from child memory |
| `print_return_value()` | output.c | Formats errno codes |

**Data Flow:**

```
Child Process (SIGTRAP)
    ↓
get_syscall_id() + get_syscall_args()
    ↓
output_on_entry() [saves: name, args, arg_count]
    ↓
[syscall executes]
    ↓
get_syscall_return()
    ↓
output_on_exit() [prints formatted line]
    ↓
Display: write(1, "hello\n", 6) = 6
```

---

### 🧪 Member 5: Integration & Testing (Mariam)

**Focus:** Build System, Error Handling, Quality Assurance

**Build Configuration:**
```makefile
CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/decoder.c src/registers.c src/syscalls.c src/formatter.c src/output.c
TARGET = strace_tracer
```

**Error Handling:** Added checks for all ptrace calls:
- `fork()` / `PTRACE_TRACEME`
- `PTRACE_SETOPTIONS` / `PTRACE_SYSCALL`
- `waitpid()` for process synchronization
- `PTRACE_GETREGS()` (×3 calls)
- `PTRACE_PEEKDATA()` for memory access

---

## ✅ Testing & Validation

### Test Suite

| # | Program | Status | Key Output |
|---|---------|--------|-----------|
| 1 | `ls` | ✅ Pass | `write(1, "Makefile   docs...", 48) = 48` |
| 2 | `echo hello` | ✅ Pass | `write(1, "hello\n", 6) = 6` |
| 3 | `cat /etc/hostname` | ✅ Pass | `write(1, "LAPTOP-EG9CRD6K\n", 16) = 16` |
| 4 | `pwd` | ✅ Pass | `write(1, "/root/System-Call-Tracer-1\n", 27) = 27` |
| 5 | `whoami` | ✅ Pass | `write(1, "root\n", 5) = 5` |

**Full Report:** See `docs/TEST_REPORT.md`

---

## 📚 Additional Documentation

Detailed technical documentation for each team member:

- **[MEMBER1-core-engine.md](docs/MEMBER1-core-engine.md)** — ptrace architecture & dual-stop logic
- **[MEMBER2-decoder.md](docs/MEMBER2-decoder.md)** — Syscall mapping & lookup table design
- **[MEMBER3-registers.md](docs/MEMBER3-registers.md)** — x86-64 register extraction
- **[MEMBER4-output-formatter.md](docs/MEMBER4-output-formatter.md)** — Output formatting & string handling
- **[MEMBER5-testing.md](docs/MEMBER5-testing.md)** — Test cases & error handling
- **[TEST_REPORT.md](docs/TEST_REPORT.md)** — Complete validation results

---

## 🎓 Learning Outcomes

Building this tracer taught us:

- ✅ How ptrace() enables process debugging at the syscall level
- ✅ x86-64 calling conventions and ABI specifications
- ✅ Process synchronization with fork/exec/waitpid
- ✅ Low-level memory access and register inspection
- ✅ Error handling in systems programming
- ✅ Collaborative software development practices

---

## 📝 Notes

- This project is **educational** and demonstrates ptrace concepts
- For production tracing, use the real `strace` utility
- Requires **Linux** (ptrace is not available on macOS/Windows natively)
- Full source available on **GitHub**: [Abouzeidd/System-Call-Tracer](https://github.com/Abouzeidd/System-Call-Tracer.git)

---

**Last Updated:** Spring 2026 | **Course:** Operating Systems (OS)
