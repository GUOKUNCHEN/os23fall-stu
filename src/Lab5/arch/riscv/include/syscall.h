#define SYS_WRITE   64
#define SYS_GETPID  172

uint64 sys_write(unsigned int fd, const char* buf, size_t count);
uint64 sys_getpid();
void syscall(struct pt_regs* regs);