#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define PORT 5555

int validatePassword(char *pwd) {
    int len = strlen(pwd);
    if(len < 8 || len > 20) return 0;

    int hasUpper=0, hasLower=0, hasDigit=0, hasSpecial=0;
    for(int i=0; i<len; i++) {
        if(islower(pwd[i])) hasLower=1;
        else if(isupper(pwd[i])) hasUpper=1;
        else if(isdigit(pwd[i])) hasDigit=1;
        else if(pwd[i]=='_' || pwd[i]=='@' || pwd[i]=='$') hasSpecial=1;
    }

    return (hasUpper && hasLower && hasDigit && hasSpecial);
}

int main() {
    int sockfd, newSocket;
    struct sockaddr_in serverAddr, newAddr;
    socklen_t addr_size;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Socket error");
        exit(1);
    }
    printf("Password server socket created.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind error");
        exit(1);
    }
    printf("Bind to port %d\n", PORT);

    if(listen(sockfd, 10) == 0) {
        printf("Listening...\n");
    } else {
        printf("Listen error\n");
    }

    while(1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if(newSocket < 0) {
            perror("Accept error");
            exit(1);
        }
        printf("Client connected.\n");

        recv(newSocket, buffer, 1024, 0);
        if(validatePassword(buffer)) {
            strcpy(buffer, "Password is VALID ✅");
        } else {
            strcpy(buffer, "Password is INVALID ❌");
        }
        send(newSocket, buffer, strlen(buffer)+1, 0);
        close(newSocket);
    }

    return 0;
}
