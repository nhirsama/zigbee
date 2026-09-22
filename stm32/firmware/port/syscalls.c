#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include "stm32f10x.h"

extern char _end;
extern char _estack;

void *_sbrk(ptrdiff_t incr)
{
    static char *heap_end;
    char *prev_heap_end;
    char *stack;

    if (heap_end == 0) {
        heap_end = &_end;
    }

    __asm volatile ("mrs %0, msp" : "=r" (stack));
    if (heap_end + incr > stack) {
        errno = ENOMEM;
        return (void *)-1;
    }

    prev_heap_end = heap_end;
    heap_end += incr;
    return prev_heap_end;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {}
        USART_SendData(USART1, (uint8_t)ptr[i]);
    }
    return len;
}

int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _close(int file) { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
void _exit(int status) { (void)status; while (1) {} }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void) { return 1; }
void _init(void) {}
void _fini(void) {}
