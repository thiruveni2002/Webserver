#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdlib.h>
#include <signal.h>
#include <process.h>
#include "./utils.h"

#pragma comment(lib, "ws2_32.lib")    

#define PORT 8080
SOCKET serverSocket;

int main() {
    WSADATA wsaData;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    //Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }
    //create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;   //set the sckt
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    //Puts the socket in listening mode
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is running on port %d...\n", PORT);
    signal(SIGINT, handleShutdown);
    
    //accept and handle clients
    while (1) {
        SOCKET *clientSocket = malloc(sizeof(SOCKET));
        if (!clientSocket) {
            continue;
        }

        *clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (*clientSocket == INVALID_SOCKET) {
            free(clientSocket);
            continue;
        }

        _beginthread(handleClientRequest, 0, clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
