#include <stdio.h>
#include <stdint.h>  // fixed-width int type
#include <string.h>
#include <winsock2.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>  
#include "utils.h"

#define BUFSIZE 4096
#define DOCUMENT_ROOT "src"

extern SOCKET serverSocket;

//determines the MIME type of a requested file
const char* getContentType(const char* filePath) {
    if (strstr(filePath, ".html")) return "text/html";
    if (strstr(filePath, ".css")) return "text/css";
    if (strstr(filePath, ".js")) return "application/javascript";
    if (strstr(filePath, ".png")) return "image/png";
    if (strstr(filePath, ".jpg") || strstr(filePath, ".jpeg")) return "image/jpeg";
     if (strstr(filePath, ".pdf")) return "application/pdf"; 
     if (strstr(filePath, ".mp4")) return "video/mp4"; 
    if (strstr(filePath, ".mp3")) return "audio/mpeg"; 
    return "application/octet-stream";
}

void handleClientRequest(void* clientSocketPtr) {
    SOCKET clientSocket = *(SOCKET*)clientSocketPtr;
    free(clientSocketPtr);

    char buffer[BUFSIZE];
    int recvSize = recv(clientSocket, buffer, BUFSIZE - 1, 0);
    if (recvSize <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[recvSize] = '\0';
    printf("Received request:\n%s\n", buffer);

    char *method = strtok(buffer, " ");
    char *filePath = strtok(NULL, " ");

    if (!method || !filePath || strcmp(method, "GET") != 0) {
        const char *badRequestResponse = "HTTP/1.1 400 Bad Request\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Connection: close\r\n\r\n"
                                         "<html><body><h1>400 Bad Request</h1></body></html>";
        send(clientSocket, badRequestResponse, strlen(badRequestResponse), 0);
        closesocket(clientSocket);
        return;
    }

    // Default to index.html if path is "/"
    if (strcmp(filePath, "/") == 0) {
        filePath = "index.html";
    }

    // Handle the static file request (e.g., CSS, JS)
    char fullPath[BUFSIZE];
    _snprintf(fullPath, BUFSIZE, "%s/%s", DOCUMENT_ROOT, filePath);


    // Debugging output for full path
    printf("Full file path: %s\n", fullPath);

    FILE *file = fopen(fullPath, "rb");
    if (!file) {
        const char *notFoundResponse = "HTTP/1.1 404 Not Found\r\n"
                                       "Content-Type: text/html\r\n"
                                       "Connection: close\r\n\r\n"
                                       "<html><body><h1>404 Not Found</h1></body></html>";
        send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
        closesocket(clientSocket);
        return;
    }

    const char *contentType = getContentType(filePath);
    
    
    if (strcmp(contentType, "application/pdf") == 0) {
        snprintf(buffer, sizeof(buffer),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Disposition: attachment; filename=\"download.pdf\"\r\n"
                 "Connection: close\r\n\r\n",
                 contentType);
    } else {
        snprintf(buffer, sizeof(buffer),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Connection: close\r\n\r\n",
                 contentType);
    }

    send(clientSocket, buffer, strlen(buffer), 0);

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFSIZE, file)) > 0) {
        send(clientSocket, buffer, bytesRead, 0);
    }

    fclose(file);
    closesocket(clientSocket);
}


void startServer() {
    SOCKET clientSocket;
    while (1) {
        clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed\n");
            continue;
        }

        SOCKET *clientSockPtr = (SOCKET*)malloc(sizeof(SOCKET));
        if (!clientSockPtr) {
            printf("Memory allocation failed\n");
            closesocket(clientSocket);
            continue;
        }

        *clientSockPtr = clientSocket;

        
        uintptr_t threadHandle = _beginthread(handleClientRequest, 0, clientSockPtr);
        if (threadHandle == -1L) {
            printf("Thread creation failed\n");
            free(clientSockPtr);
            closesocket(clientSocket);
        }
    }
}

void handleShutdown(int signum) {
	    (void)signum;
    printf("\nShutting down server...\n");
    closesocket(serverSocket);
    WSACleanup();
    exit(0);
}

