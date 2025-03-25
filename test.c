#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 8192
#define NUM_CONCURRENT_REQUESTS 10


double measureRequestTime(const char* request) {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    clock_t start, end;
    double time_taken;

    
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return -1;
    }

    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Measure start time
    start = clock();

    
    send(clientSocket, request, strlen(request), 0);

    
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; 
    }

    // Measure end time
    end = clock();

    
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    
    closesocket(clientSocket);
    WSACleanup();

    return time_taken;
}

void testConcurrentRequests() {
    printf("Test: Handling concurrent requests...\n");

    
    for (int i = 0; i < NUM_CONCURRENT_REQUESTS; i++) {
        // Perform a request for each concurrent test
        double timeTaken = measureRequestTime("GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");

        if (timeTaken >= 0) {
            printf("Request %d response time: %.4f seconds\n", i + 1, timeTaken);
        }
    }
}

void sendRequest(const char* request, const char* expectedResponse) {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return;
    }

    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    
    send(clientSocket, request, strlen(request), 0);

    
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Response:\n%s\n", buffer);

        
        if (strstr(buffer, expectedResponse)) {
            printf("? Test Passed\n");
        } else {
            printf("? Test Failed\n");
        }
    }

    
    closesocket(clientSocket);
    WSACleanup();
}

int main() {
    
    printf("Test 1: Server is running and serves a default page...\n");
    sendRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n", "200 OK");

    
    printf("Test 2: Server serves index.html...\n");
    sendRequest("GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n", "200 OK");

    
    printf("Test 3: Server serves CSS file...\n");
    sendRequest("GET /css/style.css HTTP/1.1\r\nHost: localhost\r\n\r\n", "text/css");

    
    printf("Test 4: Server serves JS file...\n");
    sendRequest("GET /js/index.js HTTP/1.1\r\nHost: localhost\r\n\r\n", "application/javascript");

    
    printf("Test 5: Server serves image file...\n");
    sendRequest("GET /images/logo.png HTTP/1.1\r\nHost: localhost\r\n\r\n", "image/png");

    
    printf("Test 6: Server serves PDF file...\n");
    sendRequest("GET /tutorial.pdf HTTP/1.1\r\nHost: localhost\r\n\r\n", "200 OK");
    sendRequest("GET /tutorial.pdf HTTP/1.1\r\nHost: localhost\r\n\r\n", "application/pdf");

    
    printf("Test 7: Error handling for non-existing file...\n");
    sendRequest("GET /nonexistent.html HTTP/1.1\r\nHost: localhost\r\n\r\n", "404 Not Found");

    
    testConcurrentRequests();

    return 0;
}

