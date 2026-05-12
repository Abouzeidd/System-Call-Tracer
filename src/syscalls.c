#include <stddef.h>
#include "syscalls.h"

static syscall_meta syscall_table[] = {

    {0, "read", 3},
    {1, "write", 3},
    {2, "open", 3},
    {3, "close", 1},
    {39, "getpid", 0},
    {57, "fork", 0},
    {59, "execve", 3},
    {60, "exit", 1},
    {62, "kill", 2},
    {257, "openat", 4},
    {5, "fstat", 2},
    {9, "mmap", 6},
    {11, "munmap", 2},
    {12, "brk", 1},
    {21, "access", 2},
    {22, "pipe", 1},
    {32, "dup", 1},
    {33, "dup2", 2},
    {41, "socket", 3},
    {42, "connect", 3}
};

const syscall_meta* get_syscall(int id)
{
    int size =
        sizeof(syscall_table)
        / sizeof(syscall_meta);

    for(int i = 0; i < size; i++)
    {
        if(syscall_table[i].id == id)
        {
            return &syscall_table[i];
        }
    }

    return NULL;
}

