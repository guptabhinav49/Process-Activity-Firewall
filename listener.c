#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include "connection.h"

int main(int argc, char *argv[]){
    struct sockaddr_un addr;
    char buf[MAX_BUFFLEN];
    int sfd, cfd, nbytes;

    // Creating a server socket, this is where this application listens for incoming log buffers
    if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    // Setting the family and path, this path is available to the logger function as well
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path));

    // unlinking the path before binding it to the socket
    unlink(SV_SOCK_PATH);

    // binding the socket with a known address
    if(bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind error");
        exit(-1);
    }

    // making the socket passive *listener*
    if(listen(sfd, 100) == -1) {
        perror("listener error");
        exit(-1);
    }

    // accepting connections from the active sockets
    printf("Accepting active sockets...\n");
    while(1){
        if((cfd = accept(sfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }

        while((nbytes=read(cfd, buf, MAX_BUFFLEN)) > 0) {
            // printing the logged buffer
            printf("%s", buf);
        }
    }
}