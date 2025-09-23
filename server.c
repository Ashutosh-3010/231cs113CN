#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
int client_count = 0;
int peer_conn = -1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast(char* msg, int sender) {
    pthread_mutex_lock(&lock);
    for(int i = 0; i < client_count; i++) {
        if(clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    while(1) {
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE-1, 0);
        if(bytes_received <= 0) break;
        
        buffer[bytes_received] = '\0';
        printf("From client %d: %s\n", client_sock, buffer);
        
        // Send to local clients
        broadcast(buffer, client_sock);
        
        // Forward to peer server
        if(peer_conn != -1) {
            send(peer_conn, buffer, strlen(buffer), 0);
        }
    }
    
    pthread_mutex_lock(&lock);
    for(int i = 0; i < client_count; i++) {
        if(clients[i] == client_sock) {
            clients[i] = clients[client_count-1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    
    close(client_sock);
    return NULL;
}

void* handle_peer(void* arg) {
    int peer_sock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    while(1) {
        int bytes_received = recv(peer_sock, buffer, BUFFER_SIZE-1, 0);
        if(bytes_received <= 0) break;
        
        buffer[bytes_received] = '\0';
        printf("From peer: %s\n", buffer);
        broadcast(buffer, -1);
    }
    
    peer_conn = -1;
    close(peer_sock);
    return NULL;
}

void start_server(int my_port, int peer_port) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(my_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);
    
    printf("Server listening on port %d\n", my_port);
    
    // Connect to peer if given
    if(peer_port != -1) {
        peer_conn = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in peer_addr;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer_port);
        peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if(connect(peer_conn, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == 0) {
            pthread_t peer_thread;
            pthread_create(&peer_thread, NULL, handle_peer, &peer_conn);
            printf("Connected to peer on port %d\n", peer_port);
        } else {
            printf("Could not connect to peer.\n");
            peer_conn = -1;
        }
    }
    
    while(1) {
        int client_sock = accept(server_sock, NULL, NULL);
        
        pthread_mutex_lock(&lock);
        if(client_count < MAX_CLIENTS) {
            clients[client_count++] = client_sock;
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handle_client, &client_sock);
            printf("Client connected. Total clients: %d\n", client_count);
        } else {
            close(client_sock);
        }
        pthread_mutex_unlock(&lock);
    }
    
    close(server_sock);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s my_port [peer_port]\n", argv[0]);
        return 1;
    }
    
    int my_port = atoi(argv[1]);
    int peer_port = (argc > 2) ? atoi(argv[2]) : -1;
    
    start_server(my_port, peer_port);
    return 0;
}
