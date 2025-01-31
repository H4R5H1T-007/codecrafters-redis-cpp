#pragma once
#include <vector>
#include <string>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct clientContext{
    int fd;
    std::string writeContext;
    std::string writeBeforeRESP;
    std::vector<std::string> readArguments;
    clientContext(int fd): fd(fd)
    {
        writeContext = "*0\r\n";
    }
};

extern struct serverContext{
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    int server_fd;
} sContext;

// Function declarations
bool parseRESPInput(char *&buffer, int &length, int client_fd);
bool pollServer();
bool initServerConnection();
bool createBulkRESPString(int client_location);
bool writeToClientBuffer(int client_location);

// If you need to share variables between files
extern std::vector<clientContext> cContext;  // Declaration only