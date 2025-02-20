#pragma once
#include <vector>
#include <string>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory>

struct clientContext{
    // int fd;
    pollfd clientFD;
    std::string writeContext;
    std::vector<std::string> writeBeforeRESP;
    std::vector<std::string> readArguments;
    clientContext(int fd): clientFD{fd, POLLIN | POLLPRI}
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
bool parseRESPInput(char *&buffer, int &length, std::shared_ptr<clientContext> & clientData);
bool pollServer();
bool initServerConnection();
bool createBulkRESPString(std::shared_ptr<clientContext> clientData);
bool nullBulkRESPString(std::shared_ptr<clientContext> clientData);
bool createSimpleRESPString(std::shared_ptr<clientContext> clientData);
bool createSimpleErrorString(std::shared_ptr<clientContext> clientData);
bool writeToClientBuffer(std::shared_ptr<clientContext> clientData);
bool clearClientContext(std::shared_ptr<clientContext> clientData);

// If you need to share variables between files
extern std::vector<std::shared_ptr<clientContext>> cContext;  // Declaration only