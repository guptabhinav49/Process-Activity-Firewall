#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include "headers.h"
#include "lwjson/lwjson.h"

// Implementing the KMP algorithm, for substring matching
int match(char a[], const char sub[]){
    // printf("%s %s %lu %lu\n", a, sub, strlen(a), strlen(sub));
    if(strlen(a) < strlen(sub)) return 0;

    char b[MAX_SUBSTRLEN_TO_CHECK];
    strncpy(b, sub, sizeof(b));
    strncat(b, "~", 1);

    size_t prefix_match[strlen(b)];

    prefix_match[0] = 0;

    for(int i=1; i<strlen(b); i++){
        int j = prefix_match[i-1];

        while(j>0 && b[i] != b[j]) j = prefix_match[j-1];

        if(b[i] == b[j]) j++;
        prefix_match[i] = j;
    }

    size_t last = prefix_match[strlen(b)-1];
    int ans = 0;

    for(int i = 0; i<strlen(a); i++){
        while(last > 0 && a[i] != b[last]) last = prefix_match[last-1];

        if(a[i] == b[last]) last++;

        if(last == strlen(b)-1){
            ans = 1;
            break;
        }
    }
    return ans;
}

int main(int argc, char *argv[]){

    if(argc > 2){
        printf("Usage: %s <substring>", argv[1]);
        return 1;
    }

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
    printf("Process-Activity-Forewall\n\n");
    while(1){
        if((cfd = accept(sfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }

        while((nbytes=read(cfd, buf, MAX_BUFFLEN)) > 0) {
            // printing the logged buffer
            if(argc == 1)            
                printf("%s", buf);
            else if (argc == 2){
                // printf("matching substring\n");
                if(match(buf, argv[1])){
                    printf("%s", buf);
                }
                // else{
                //     printf("NO MATCH!\n");
                // }
            }
        }
        close(cfd);
    }
}