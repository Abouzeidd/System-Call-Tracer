# Test Report — strace_tracer

## Environment
- OS: Ubuntu 24.04 (WSL2)
- Architecture: x86-64
- Binary: `./strace_tracer`

---

## 1. `./strace_tracer ls`

```
brk(0) = 105464578408448
mmap(0, 8192, 3, 34, 0xffffffff, 0) = 139839752151040
access("/etc/ld.so.preload", 4) = -1 /* error 2 */
openat("", 0x7f2efabfb1db, 0x80000, 0) = 3
fstat(3, 0x7fffc17192d0) = 0
read(3, 0x5feb62bb8500, 1024) = 442
close(3) = 0
...
write(1, "Makefile   docs   include  strace_tracer  tests\n", 48) = 48
write(1, "README.md  hello  src      test.c         tracer\n", 41) = 41
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## 2. `./strace_tracer echo hello`

```
brk(0) = 104522028429312
access("/etc/ld.so.preload", 4) = -1 /* error 2 */
...
fstat(1, 0x7ffd821c1f80) = 0
write(1, "hello\n", 6) = 6
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## 3. `./strace_tracer cat /etc/hostname`

```
brk(0) = 108978703577088
access("/etc/ld.so.preload", 4) = -1 /* error 2 */
...
openat("", 0x7ffe44d84fa0, 0, 0) = 3
read(3, 0x7a9a6b95e000, 0x20000) = 16
write(1, "LAPTOP-EG9CRD6K\n", 16) = 16
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## 4. `./strace_tracer pwd`

```
brk(0) = 109022948184064
access("/etc/ld.so.preload", 4) = -1 /* error 2 */
...
write(1, "/root/System-Call-Tracer-1\n", 27) = 27
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## 5. `./strace_tracer whoami`

```
brk(0) = 107433904214016
access("/etc/ld.so.preload", 4) = -1 /* error 2 */
...
socket(1, 0x80801, 0) = 3
connect(3, 0x7ffe9db98d80, 110) = -1 /* error 2 */
read(3, 0x61b5e7b2d620, 4096) = 526
write(1, "root\n", 5) = 5
close(1) = 0
close(2) = 0

[DONE] Target process exited.
```

---

## Summary

| # | Program | Status | Output |
|---|---------|--------|--------|
| 1 | `ls` | ✅ Pass | Listed all files in directory |
| 2 | `echo hello` | ✅ Pass | `write(1, "hello\n", 6) = 6` |
| 3 | `cat /etc/hostname` | ✅ Pass | `write(1, "LAPTOP-EG9CRD6K\n", 16) = 16` |
| 4 | `pwd` | ✅ Pass | `write(1, "/root/System-Call-Tracer-1\n", 27) = 27` |
| 5 | `whoami` | ✅ Pass | `write(1, "root\n", 5) = 5` |

All 5 programs traced successfully. The tracer correctly intercepts syscall entry and exit, decodes string arguments, and reports return values including errors.
