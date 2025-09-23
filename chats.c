#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // ADD THIS LINE

int clients[10];
int count = 0;
int peer_sock = -1;

void send_to_all(char* msg, int skip) {
    for(int i = 0; i < count; i++) {
        if(clients[i] != skip) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Use: %s my_port [peer_port]\n", argv[0]);
        return 1;
    }
    
    int my_port = atoi(argv[1]);
    int peer_port = (argc > 2) ? atoi(argv[2]) : -1;
    
    // Setup server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(my_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("Server on port %d\n", my_port);
    
    // Connect to peer
    if(peer_port != -1) {
        peer_sock = socket(AF_INET, SOCK_STREAM, 0);
        
        struct sockaddr_in peer_addr;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer_port);
        peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if(connect(peer_sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == 0) {
            printf("Connected to peer %d\n", peer_port);
        }
    }
    
    while(1) {
        int client_fd = accept(server_fd, NULL, NULL);
        clients[count++] = client_fd;
        printf("Client %d connected\n", count);
        
        if(fork() == 0) {  // Child process handles client
            close(server_fd);
            char msg[1000];
            
            while(1) {
                int n = recv(client_fd, msg, 999, 0);
                if(n <= 0) break;
                
                msg[n] = '\0';
                printf("Got: %s\n", msg);
                
                send_to_all(msg, client_fd);  // Send to other clients
                
                if(peer_sock != -1) {
                    send(peer_sock, msg, strlen(msg), 0);  // Send to peer server
                }
            }
            
            close(client_fd);
            exit(0);
        }
        close(client_fd);
    }
}
