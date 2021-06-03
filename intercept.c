#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <sys/stat.h>

typedef int (*orig_open2_type)(const char *pathname, int flags);
typedef int (*orig_open_type)(const char *pathname, int flags, mode_t mode);
typedef ssize_t (*orig_read_type)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_type)(int fd, const void *buf, size_t count);
typedef int (*orig_close_type)(int fd);

/*
----
    open() with only 2 parameters.
----
*/
/*
int open(const char *pathname, int flags){

    printf("open() called.\nArgs \tpath: %s \n\tflag: %d\n", pathname, flags);

    orig_open2_type orig_open;
    orig_open = (orig_open2_type)dlsym(RTLD_NEXT, "open");
    return orig_open(pathname, flags);
}
*/


int open(const char *pathname, int flags, mode_t mode){
    printf("open() called.\n Args \tpath: %s, flag: %d, mode: %04o, ", pathname, flags, mode&0777);
    orig_open_type orig_open;
    orig_open = (orig_open_type)dlsym(RTLD_NEXT, "open");

    int fd  = orig_open(pathname, flags, mode);

    printf("fd: %d\n", fd);
    printf("==========\n");
    return fd;
}

ssize_t write(int fd, const void *buf, size_t count){
    printf("write() called.\n Args \tfile: %d, buf_addr: %p, size: %zu\n", fd, buf, count);
    printf("==========\n");

    orig_write_type orig_write;
    orig_write = (orig_write_type)dlsym(RTLD_NEXT, "write");
    return orig_write(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count){
    printf("read() called.\n Args \tfile: %d, buf_addr: %p, size: %zu\n", fd, buf, count);
    printf("==========\n");

    orig_read_type orig_read;
    orig_read = (orig_read_type)dlsym(RTLD_NEXT, "read");
    return orig_read(fd, buf, count);
}

int close(int fd){
    printf("close() called.\n Args \tfile: %d\n", fd);
    printf("==========\n");

    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");
    return orig_close(fd);
}