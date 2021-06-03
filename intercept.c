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

int log_to_socket(const char *buf, size_t len, void *wrp){
    struct sockaddr_un addr;
    
    // Creating a client socket to send the input buffer to the listening server, that will be running parallely
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    // printf("Client socket fd = %d\n", sfd);

    // Making sure the socket is allocated
    if(sfd == -1){
        perror("socket error");
        return -1;
    }

    // Adding the server address stored in a macro (available from the header file "connection.h") to the struct addr
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path)-1);

    // connecting the active socket sfd to the listening socket whose addrress is stored in addr
    if(connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        return -1;
    }

    // copy the buffer to socket for the server to read
    int try = 10;
    int done = 0;
    while(try){
        
        if(write(sfd, buf, len+1) == len+1){
            // printf("writing %lu chars", len);
            done = 1;
            break;
        }
        try--;
    }

    // return error if write to socket is not successful
    if(done==0){
        perror("write/partial write");
        return -1;
    }

    close(sfd);
    return 0;
}

int isitSocket(char *str){
    char *s = "socket";
    int same = 1;
    for(int i=0; i<5; i++){
        if(s[i]!=str[i]){
            same = 0;
            break;
        }
    }
    return same;
}

int open(const char *pathname, int flags, mode_t mode){
    // maintaining the logging info in the character string buf
    char buf[MAX_BUFFLEN], temp[100];

    // adding the parameter values to the buffer
    sprintf(buf, "open() called. Params are\n \tpath: %s, flag: %d, mode: %04o, ", pathname, flags, mode&0777);
    // strncat(buf, temp, sizeof(temp));

    // fetching, and then calling the original open
    orig_open_type orig_open;
    orig_open = (orig_open_type)dlsym(RTLD_NEXT, "open");
    int fd  = orig_open(pathname, flags, mode);

    // adding the returned file descriptor in the logging info
    sprintf(temp, "fd: %d\n\n", fd);
    strncat(buf, temp, sizeof(temp));
    
    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf), NULL) != 0){
        perror("logging to socket failed");
    }

    return fd;
}

ssize_t write(int fd, const void *buffer, size_t count){
    // setting the getfrom string to get the path from the file descriptor
    char getfrom[50], path[MAX_PATHLEN], temp[100], buf[MAX_BUFFLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // adding the parameter values to buffer
    sprintf(buf, "write() called. Params are\n \tfile: %.*s, buf_addr: %p, size: %zu\n", (int)nbytes, path, buffer, count);

    // checking whether we got the full file path or not
    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "file path may have been truncated!");
        strncat(buf, temp, sizeof(temp));
    }
    strncat(buf, "\n", 1);

    // calling the original write() function with same parameters
    orig_write_type orig_write;
    orig_write = (orig_write_type)dlsym(RTLD_NEXT, "write");

    // **** HACK to resolve the recursive calls to the write() on writing to a socket (probably need a better workaround) ****
    if(isitSocket(path) == 1){
        return orig_write(fd, buffer, count);
    }

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf), orig_write) != 0){
        perror("logging to socket failed");
    }

    return orig_write(fd, buffer, count);
}

ssize_t read(int fd, void *buffer, size_t count){
    // setting getfrom string to get the path from the file descriptor
    char getfrom[50], path[MAX_PATHLEN], temp[100], buf[MAX_BUFFLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);
    
    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // logging the parameters
    sprintf(buf, "read() called. Params are\n \tfile: %.*s, buf_addr: %p, size: %zu\n", (int)nbytes, path, buffer, count);
    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "file path may have been truncated!");
        strncat(buf, temp, sizeof(temp));
    }
    strncat(buf, "\n", 1);

    // calling the original read()
    orig_read_type orig_read;
    orig_read = (orig_read_type)dlsym(RTLD_NEXT, "read");

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf), NULL) != 0){
        perror("logging to socket failed");
    }

    return orig_read(fd, buffer, count);
}

int close(int fd){
    // setting getfrom string to get the path from the file descriptor
    char getfrom[50], path[MAX_PATHLEN], buf[MAX_BUFFLEN], temp[100];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // logging params to the buffer
    sprintf(buf, "close() called. Params are\n \tfile: %.*s\n", (int)nbytes, path);

    // checking whether we got the full filename or not
    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "file path may have been truncated!");
        strncat(buf, temp, sizeof(temp));
    }
    strncat(buf, "\n", 1);

    // calling the original close
    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");

    // **** HACK to resolve the recursive calls to the close() on writing to a socket (probably need a better workaround) ****
    if(isitSocket(path) == 1){
        return orig_close(fd);
    }

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf), NULL) != 0){
        perror("logging to socket failed");
    }

    return orig_close(fd);
}