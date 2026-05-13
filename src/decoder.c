#include <stddef.h>
#include "tracer.h"
#include "syscalls.h"

<<<<<<< HEAD
const char* get_syscall_name(long syscall_id) {
    const syscall_meta *meta = get_syscall((int)syscall_id);
    if (meta != NULL) {
        return meta->name;
    }
    return "UNKNOWN";
}
=======
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


>>>>>>> 41f35df8639d1b0237d0a4d13719579ce0fac94b
