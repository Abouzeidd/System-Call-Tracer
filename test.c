#include <stdio.h>
#include "syscalls.h"

int main()
{
    const syscall_meta *meta;

    meta = get_syscall(59);

    if(meta != NULL)
    {
        printf("Syscall Name: %s\n", meta->name);
        printf("Arguments: %d\n", meta->argc);
    }

    return 0;
}

