# System Call Tracer (strace-clone)
A lightweight Linux utility designed to intercept and record system calls made by a process. Developed as a group project for the Operating Systems course.

## 👥 The Team
* **Abdelrahman Abouzeid** - Team Lead & Core Engine Logic
* **Malak Amir** - Syscall Decoder & Name Mapping
* **Alaa Tamer** - Register Extraction & CPU Logic
* **Mohamed Osama** - Error Handling & Edge Cases
* **Mariam Mazen** - Documentation & Testing Suite

## 🚀 Quick Start
### Prerequisites
* Linux Environment (Ubuntu/WSL2 recommended)
* GCC Compiler
* Make build tool

### Installation & Build
1. Clone the repository:
   ```bash
   git clone [https://github.com/Abouzeidd/System-Call-Tracer.git](https://github.com/Abouzeidd/System-Call-Tracer.git)
   cd System-Call-Tracer

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

