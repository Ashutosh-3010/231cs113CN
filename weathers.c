#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 6666

// Hardcoded weather database
struct Weather {
    char city[50];
    char condition[50];
    int temperature;
    int humidity;
};

struct Weather db[] = {
    {"mumbai", "Sunny", 32, 70},
    {"delhi", "Cloudy", 28, 60},
    {"bangalore", "Rainy", 24, 85},
    {"chennai", "Humid", 34, 80},
    {"kolkata", "Stormy", 26, 90}
};

char* getWeatherReport(char *city) {
    static char report[256];
    for(int i=0; i<sizeof(db)/sizeof(db[0]); i++) {
        if(strcasecmp(city, db[i].city) == 0) {
            sprintf(report, "Weather Report for %s:\nCondition: %s\nTemperature: %d C\nHumidity: %d%%",
                    db[i].city, db[i].condition, db[i].temperature, db[i].humidity);
            return report;
        }
    }
    sprintf(report, "Weather data for '%s' not found!", city);
    return report;
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
    printf("Weather server socket created.\n");

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
        printf("Listening for clients...\n");
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
        printf("Received city request: %s\n", buffer);

        char *report = getWeatherReport(buffer);
        send(newSocket, report, strlen(report)+1, 0);

        close(newSocket);
        printf("Client disconnected.\n");
    }

    return 0;
}
