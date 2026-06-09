
#include "syscall.h"

__attribute__((weak)) void _write_char(char c)
{
    (void)c;
}

__attribute__((weak)) int _write(int fd, const void *buf, size_t count)
{
    (void)fd;
    const char *cbuf = buf;
    for (size_t i = 0; i < count; i++) {
        _write_char(cbuf[i]);
    }
    return (int)count;
}

__attribute__((weak)) int _read(int fd, void *buf, size_t count)
{
    (void)fd; (void)buf; (void)count;
    errno = ENOSYS;
    return -1;
}

__attribute__((weak)) int _close(int fd)
{
    (void)fd;
    errno = ENOSYS;
    return -1;
}

__attribute__((weak)) off_t _lseek(int fd, off_t offset, int whence)
{
    (void)fd; (void)offset; (void)whence;
    errno = ENOSYS;
    return -1;
}

__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR; // pretend it's a character device
    return 0;
}

__attribute__((weak)) int _isatty(int fd)
{
    (void)fd;
    return 1;
}

// Optional: only if you use malloc()
extern char _end;   // provided by linker
static char *heap_end;

__attribute__((weak)) void *_sbrk(ptrdiff_t incr)
{
    if (!heap_end)
        heap_end = &_end;
    char *prev = heap_end;
    heap_end += incr;
    return prev;
}

