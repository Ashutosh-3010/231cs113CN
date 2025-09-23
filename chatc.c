#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // ADD THIS LINE

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Use: %s ip port\n", argv[0]);
        return 1;
    }
    
    // Connect to server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Can't connect\n");
        return 1;
    }
    
    printf("Connected! Type messages:\n");
    
    if(fork() == 0) {  // Child process receives messages
        char msg[1000];
        while(1) {
            int n = recv(sock, msg, 999, 0);
            if(n <= 0) break;
            msg[n] = '\0';
            printf(">> %s\n", msg);
        }
        exit(0);
    }
    
    // Parent process sends messages
    char msg[1000];
    while(1) {
        fgets(msg, 1000, stdin);
        msg[strcspn(msg, "\n")] = '\0';  // Remove newline
        
        if(strcmp(msg, "/quit") == 0) break;
        
        send(sock, msg, strlen(msg), 0);
    }
    
    close(sock);
    return 0;
}
