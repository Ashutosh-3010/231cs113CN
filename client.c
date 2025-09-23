#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void* receive_messages(void* arg) {
    int sock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    while(1) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE-1, 0);
        if(bytes_received <= 0) {
            printf("Disconnected from server\n");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s server_ip server_port\n", argv[0]);
        return 1;
    }
    
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        return 1;
    }
    
    printf("Connected to server. Type messages and press Enter.\n");
    
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &sock);
    
    char buffer[BUFFER_SIZE];
    while(1) {
        if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
        
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
        
        if(strcmp(buffer, "/quit") == 0) break;
        
        send(sock, buffer, strlen(buffer), 0);
    }
    
    close(sock);
    return 0;
}
