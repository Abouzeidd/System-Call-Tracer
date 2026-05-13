#include <stddef.h>
#include "tracer.h"
#include "syscalls.h"

const char* get_syscall_name(long syscall_id) {
    const syscall_meta *meta = get_syscall((int)syscall_id);
    if (meta != NULL) {
        return meta->name;
    }
    return "UNKNOWN";
}