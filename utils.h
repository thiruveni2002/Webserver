#ifndef UTILS_H
#define UTILS_H

#include <winsock2.h>
#include <windows.h>  // Required for WINAPI and DWORD

// Function declarations
const char* getContentType(const char* filePath);
void handleClientRequest(void* clientSocketPtr);
  // Updated to match definition
void handleShutdown(int signum);

// Global variables
extern SOCKET serverSocket;

#endif // UTILS_H
