#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 6666

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0) {
        perror("Socket error");
        exit(1);
    }
    printf("Weather client socket created.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connect error");
        exit(1);
    }
    printf("Connected to server.\n");

    printf("Enter city name: ");
    fgets(buffer, 1024, stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    send(clientSocket, buffer, strlen(buffer)+1, 0);
    recv(clientSocket, buffer, 1024, 0);

    printf("\n--- Weather Report ---\n%s\n", buffer);

    close(clientSocket);
    return 0;
}
