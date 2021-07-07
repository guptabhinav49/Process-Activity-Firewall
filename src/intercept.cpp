#include <dlfcn.h>
#include <headers.hpp>

// typedef int (*orig_open2_type)(const char *pathname, int flags);
typedef int (*orig_open_type)(const char *pathname, int flags, ...);
typedef ssize_t (*orig_read_type)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_type)(int fd, const void *buf, size_t count);
typedef int (*orig_close_type)(int fd);

/*
    cmd: 
        open() -> 0
        write() -> 1
        read() -> 2
        close() -> 3
*/
const char* get_metadata(int cmd, int *bytes, int *mode){
    // getting the original functions
    orig_open_type orig_open;
    orig_open = (orig_open_type)dlsym(RTLD_NEXT, "open");   
    orig_read_type orig_read;
    orig_read = (orig_read_type)dlsym(RTLD_NEXT, "read");
    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");

    // setting up the variables to store/help store the metadata
    char* buf = (char*)malloc(MAX_BUFFLEN * sizeof(char));
    char path[MAX_PATHLEN], cmdarg[MAX_PATHLEN];
    char procfile[25];

    int nbytes = readlink("/proc/self/exe", path, MAX_PATHLEN-1);
    path[nbytes] = '\0';

    // getting the PID, parent PID
    int PID = getpid();
    int PPID = getppid();
    
    // getting the command line arguments
    sprintf(procfile, "/proc/%d/cmdline", PID);
    int fd = orig_open(procfile, O_RDONLY);

    nbytes = orig_read(fd, cmdarg, MAX_PATHLEN-1);
    orig_close(fd);

    // the output from the procfile is '\0' seperated, therefore replacing it with ' '
    for(int i=0; i<nbytes; i++){
        if(!cmdarg[i]) cmdarg[i] = ' ';
    }
    cmdarg[nbytes-1]='\0';
    
    // finally dumping all the metadata in the buffer according to the function call
    sprintf(buf, "\t\t\"metadata\": {\"PID\": %d, \"PPID\": %d, \"EXE_PATH\": \"%s\", \"CMD\": \"%s\"", 
                PID, 
                PPID,
                path,
                cmdarg
            );
            
    if(cmd == 0){
        sprintf(path, ", \"MODE\": \"%o\"", *mode);
        strncat(buf, path, strlen(path));
    }
    else if(cmd == 1){
        sprintf(path, ", \"BYTES_WRITTEN\": %d", *bytes);
        strncat(buf, path, strlen(path));
    }
    else if (cmd == 2){
        sprintf(path, ", \"BYTES_READ\": %d", *bytes);
        strncat(buf, path, strlen(path));
    }
    strncat(buf, "}\n", 3);
    return buf;
}

int log_to_socket(const char *buf, size_t len){
    
    // making sure, this function doesn't call the tampered write() and close() functions, to avoid the recursive calls
    orig_write_type orig_write;
    orig_write = (orig_write_type)dlsym(RTLD_NEXT, "write");

    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");

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
    int t = 10;
    int done = 0;
    while(t){
        if(orig_write(sfd, buf, len+1) == len+1){
            // printf("writing %lu chars", len);
            done = 1;
            break;
        }
        t--;
    }
    orig_close(sfd);

    // return error if write to socket is not successful
    if(done==0){
        perror("write/partial write");
        return -1;
    }

    return 0;
}

int open(const char *pathname, int flags, ...){
    // maintaining the logging info in the character string buf
    char getfrom[50], path[MAX_PATHLEN], temp[100], buf[MAX_BUFFLEN];


    // fetching, and then calling the original open
    orig_open_type orig_open;
    orig_open = (orig_open_type)dlsym(RTLD_NEXT, "open");
    int fd  = orig_open(pathname, flags);

    sprintf(getfrom, "/proc/self/fd/%d", fd);
    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // adding the parameter values to the buffer
    sprintf(buf, "{\n\t\t\"function\": \"open\",\n\t\t\"params\": {\"file\": {\"path\": \"%.*s\", \"fd\": %d}, \"flags\": %d},\n", (int)nbytes, path, fd, flags);

    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "\t\t\"error\": \"file path may have been truncated\"\n");
        strncat(buf, temp, sizeof(temp));
    }

    // getting the open mode of the file
    int mode = 0;
    if(fd > 0) mode = fcntl(fd, F_GETFL);
    
    // concatenating the metadata for the open() call
    const char *meta = get_metadata(0, NULL, &mode);
    strncat(buf, meta, strlen(meta));
    strncat(buf, "\t}\n", 4);

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf)) != 0){
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
    sprintf(buf, "{\n\t\t\"function\": \"write\",\n\t\t\"params\": {\"file\": {\"path\": \"%.*s\", \"fd\": %d}, \"buff\": \"%p\", \"count\": %zu},\n", (int)nbytes, path, fd, buffer, count);

    // checking whether we got the full file path or not
    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "\t\t\"error\": \"file path may have been truncated\"\n");
        strncat(buf, temp, sizeof(temp));
    }

    // calling the original write() function with same parameters
    orig_write_type orig_write;
    orig_write = (orig_write_type)dlsym(RTLD_NEXT, "write");

    int writtenBytes = orig_write(fd, buffer, count);

    // appending the meta data along with params of the function
    const char *meta = get_metadata(1, &writtenBytes, NULL);
    strncat(buf, meta, strlen(meta));
    strncat(buf, "\t}\n", 4);

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf)) != 0){
        perror("logging to socket failed");
    }

    return writtenBytes;
}

ssize_t read(int fd, void *buffer, size_t count){

    // get_metadata();
    // setting getfrom string to get the path from the file descriptor
    char getfrom[50], path[MAX_PATHLEN], temp[100], buf[MAX_BUFFLEN];
    sprintf(getfrom, "/proc/self/fd/%d", fd);
    
    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // logging the parameters
    sprintf(buf, "{\n\t\t\"function\": \"read\",\n\t\t\"params\": {\"file\": {\"path\": \"%.*s\", \"fd\": %d}, \"buff\": \"%p\", \"count\": %zu},\n", (int)nbytes, path, fd, buffer, count);

    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "\t\t\"error\": \"file path may have been truncated\"\n");
        strncat(buf, temp, sizeof(temp));
    }

    // calling the original read()
    orig_read_type orig_read;
    orig_read = (orig_read_type)dlsym(RTLD_NEXT, "read");

    int readBytes = orig_read(fd, buffer, count);
    // appending the meta data along with params of the function
    const char *meta = get_metadata(2, &readBytes, NULL);
    strncat(buf, meta, strlen(meta));
    strncat(buf, "\t}\n", 4);

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf)) != 0){
        perror("logging to socket failed");
    }

    return readBytes;
}

int close(int fd){
    // get_metadata();
    // setting getfrom string to get the path from the file descriptor
    char getfrom[50], path[MAX_PATHLEN], buf[MAX_BUFFLEN], temp[100];
    sprintf(getfrom, "/proc/self/fd/%d", fd);

    ssize_t nbytes = readlink(getfrom, path, MAX_PATHLEN);

    // logging params to the buffer
    sprintf(buf, "{\n\t\t\"function\": \"close\",\n\t\t\"params\": {\"file\": {\"path\": \"%.*s\", \"fd\": %d} },\n", (int)nbytes, path, fd);


    // checking whether we got the full filename or not
    if(nbytes == MAX_PATHLEN){
        sprintf(temp, "\t\t\"error\": \"file path may have been truncated\"\n");
        strncat(buf, temp, sizeof(temp));
    }
    
    // appending the meta data along with params of the function
    const char *meta = get_metadata(3, NULL, NULL);
    strncat(buf, meta, strlen(meta));
    strncat(buf, "\t}\n", 4);

    // calling the original close
    orig_close_type orig_close;
    orig_close = (orig_close_type)dlsym(RTLD_NEXT, "close");

    // logging the info to the UNIX domain socket
    if(log_to_socket(buf, strlen(buf)) != 0){
        perror("logging to socket failed");
    }

    return orig_close(fd);
}