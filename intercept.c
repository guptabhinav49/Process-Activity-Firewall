#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "connection.h"

typedef int (*orig_open2_type)(const char *pathname, int flags);
typedef int (*orig_open_type)(const char *pathname, int flags, mode_t mode);
typedef ssize_t (*orig_read_type)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_type)(int fd, const void *buf, size_t count);
typedef int (*orig_close_type)(int fd);


int open(const char *pathname, int flags, mode_t mode){
    printf("==========\n");
    printf("open() called.\n Args \tpath: %s, flag: %d, mode: %04o, ", pathname, flags, mode&0777);
    orig_open_type orig_open;
    orig_open = (orig_open_type)dlsym(RTLD_NEXT, "open");

    int fd  = orig_open(pathname, flags, mode);

    printf("fd: %d\n", fd);
    printf("==========\n");
    return fd;
}

ssize_t write(int fd, const void *buf, size_t count){
    printf("==========\n");
    char getfrom[50], path[MAX_PATHLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);
    if(nbytes == MAX_PATHLEN){
        printf("file path may have been truncated!");
    }

    printf("write() called.\n Args \tfile: %.*s, buf_addr: %p, size: %zu\n", (int)nbytes, path, buf, count);
    printf("==========\n");

    orig_write_type orig_write;
    orig_write = (orig_write_type)dlsym(RTLD_NEXT, "write");
    return orig_write(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count){
    printf("==========\n");
    char getfrom[50], path[MAX_PATHLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);
    if(nbytes == MAX_PATHLEN){
        printf("file path may have been truncated!");
    }

    printf("read() called.\n Args \tfile: %.*s, buf_addr: %p, size: %zu\n", (int)nbytes, path, buf, count);
    printf("==========\n");

    orig_read_type orig_read;
    orig_read = (orig_read_type)dlsym(RTLD_NEXT, "read");
    return orig_read(fd, buf, count);
}

int close(int fd){
    char getfrom[50], path[MAX_PATHLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);
    if(nbytes == MAX_PATHLEN){
        printf("file path may have been truncated!");
    }

    printf("close() called.\n Args \tfile: %.*s\n", (int)nbytes, path);
    printf("==========\n");

    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");
    return orig_close(fd);
}