#include <fiber/util.h>
#include <unistd.h>
pid_t GetThreadId() { return syscall(SYS_gettid); }

u_int32_t GetFiberId() {
    // TODO
    return 0;
}
