#include <stddef.h>
#include "tracer.h"
#include "syscalls.h"

const char* get_syscall_name(long syscall_id)
{
    const syscall_meta *meta;

    meta = get_syscall(syscall_id);

    if(meta == NULL)
    {
        return "UNKNOWN";
    }

    return meta->name;
}


