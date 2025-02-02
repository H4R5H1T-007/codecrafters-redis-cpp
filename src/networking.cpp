// #include<vector>
#include "networking.hpp"
#include "eventLoop.hpp"
#include "commands.hpp"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <cstring>

std::vector<std::shared_ptr<clientContext>> cContext;
// std::vector<pollfd> clients;
serverContext sContext;

bool initServerConnection(){
    sContext.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sContext.server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 0;
    }
    
    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(sContext.server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 0;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);
    
    if (bind(sContext.server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 6379\n";
        perror("bind failed. Error");
        return 0;
    }
    
    int connection_backlog = 1000;
    if (listen(sContext.server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 0;
    }
  // auto async_accept = std::async(asyncAccept, sContext.server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    // clients.push_back({sContext.server_fd, POLLIN, 0});
    cContext.push_back(std::shared_ptr<clientContext>(new clientContext(sContext.server_fd)));
    return 1;
}

// TODO: Directly handling charater pointer. Need to handle this properly.
bool parseRESPInput(char *&input, int & input_len, std::shared_ptr<clientContext>& clientData){
    switch (*input)
    {
        case '*':
        {

            int len = 0;
            input++;
            input_len--;
            while (input_len > 0 && *input != '\r')
            {
                len = (len*10) + (*input - '0');
                input++;
                input_len--;
            }
            if(input_len <= 0){
                return 0;
            }
            input += 2;
            input_len -= 2;
            bool res = 1;
            for(int i = 0; res && i < len; i++){
                res &= parseRESPInput(input, input_len, clientData);
            }
            return res;
        }
        case '$':
        {
            input++; input_len--;
            size_t len = 0;
            while(input_len > 0 && *input != '\r'){
                len = (len * 10) + (*input - '0');
                input++; input_len--;
            }
            if(input_len <= 0){
                return 0;
            }
            input += 2; input_len -= 2;
            // cContext[client_location].readContext.append(input, len);
            // cContext[client_location].readContext.push_back(' ');
            clientData->readArguments.push_back({input, len});
            input += len + 2;
            input_len -= len + 2;
            return 1;
        }
        default:
        return 0;
    }
}

bool pollServer(){

    // for(int i = 1; i < clients.size(); i++){
    //     std::cout<<clients[i].fd<<" ";
    // }
    // int activity = poll(clients.data(), clients.size(), 0);
    bool activity = 1;
    for(auto &i : cContext){
        activity &= (poll(&i->clientFD, 1, 0) > 0);
    }
    
    // if (!activity) {
    //     std::cerr << "Poll error: " << strerror(errno) << std::endl;
    //     return 0;
    // }

    for (size_t i = 0; i < cContext.size(); ++i) {
        // std::cout << "Checking fd: " << cContext.clients[i].fd << ", revents: " << cContext.clients[i].revents << std::endl;
        
        if (cContext[i]->clientFD.revents & POLLIN) {
            if (cContext[i]->clientFD.fd == sContext.server_fd) {
                // New connection
                // std::cout << "New connection incoming\n";
                int new_socket = accept(sContext.server_fd, (struct sockaddr *)&sContext.client_addr, (socklen_t*)&sContext.client_addr_len);
                
                if (new_socket < 0) {
                    std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                } else {
                    // std::cout << "New socket accepted: " << new_socket << std::endl;
                    // clients.push_back({new_socket, POLLIN, 0});
                    cContext.push_back(std::shared_ptr<clientContext>(new clientContext(new_socket)));
                    // count++;
                }
            } else {
                // Existing connection has data
                // std::cout << "Data received on fd: " << cContext.clients[i].fd << std::endl;
                char buffer[1024];
                int valread = read(cContext[i]->clientFD.fd, buffer, sizeof(buffer));
                if (valread > 0) {
                    // std::cout << "Received " << valread << " bytes: " << buffer << std::endl;
                    // printf(buffer);
                    // printf("\n");
                    // fwrite(buffer, 1, strlen(buffer), stdout);
                    // if buffer starts with a * that means it's an array

                    // auto res = parseClientInput(buffer, valread);
                    // std::cout<<valread<<" "<<buffer<<std::endl;
                    char *temp = buffer;
                    if(!parseRESPInput(temp, valread, cContext[i])){
                        return 0;
                    }
                    // std::cout<<cContext[i].readContext<<std::endl;

                    el.fileSubmit(commandsHandler, cContext[i]);

                    // char* message = "+PONG\r\n";
                    // int sent = send(clients[i].fd, message, strlen(message), 0);
                    // std::cout << "Sent " << sent << " bytes: " << message << std::endl;
                } else if (valread == 0) {
                    // std::cout << "Client disconnected: " << cContext.clients[i].fd << std::endl;
                    // close(clients[i].fd);
                    // clients.erase(clients.begin() + i);
                    cContext.erase(cContext.begin() + i);
                    --i;
                } else {
                    // std::cerr << "Read error on socket " << cContext.clients[i].fd << ": " << strerror(errno) << std::endl;
                    // close(clients[i].fd);
                    // clients.erase(clients.begin() + i);
                    cContext.erase(cContext.begin() + i);
                    --i;
                }

            }
        } else if (cContext[i]->clientFD.revents & (POLLHUP | POLLERR)) {
            // Connection closed or error
            // std::cout << "Connection closed or error on fd: " << cContext.clients[i].fd << std::endl;
            // close(clients[i].fd);
            // clients.erase(clients.begin() + i);
            cContext.erase(cContext.begin() + i);
            --i;
        }
    }

    return 1;
}

bool createSimpleErrorString(std::shared_ptr<clientContext> clientData){
    try
    {
        /* code */
        std::ostringstream res;
        res<<'-'<<clientData->writeBeforeRESP[0]<<"\r\n";
        clientData->writeContext = res.str();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    return 1;
}

bool createSimpleRESPString(std::shared_ptr<clientContext> clientData){
    try
    {
        /* code */
        std::ostringstream res;
        res<<'+'<<clientData->writeBeforeRESP[0]<<"\r\n";
        clientData->writeContext = res.str();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    return 1;
}

bool createBulkRESPString(std::shared_ptr<clientContext> clientData){
    try
    {
        std::ostringstream res;
        if(clientData->writeBeforeRESP.size() != 0){
            res<<'$'<<clientData->writeBeforeRESP[0].size()<<"\r\n"<<clientData->writeBeforeRESP[0]<<"\r\n";
        }
        clientData->writeContext = res.str();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    
    return 1;
}

bool nullBulkRESPString(std::shared_ptr<clientContext> clientData){
    try
    {
        clientData->writeContext = "$-1\r\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    
    return 1;
}

bool readFromClientBuffer(std::shared_ptr<clientContext> clientData){
    try
    {
        int max_read_size = 1024;
        char buffer[max_read_size];
        auto client_fd = clientData->clientFD.fd;
        int bufferLen = read(client_fd, buffer, max_read_size);
        char * temp = buffer;
        if(bufferLen > 0){
            if(!parseRESPInput(temp, bufferLen, clientData)){
                return 0;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    
    return 1;
}

bool writeToClientBuffer(std::shared_ptr<clientContext> clientData){
    bool res = 1;
    try
    {
        int sent = send(clientData->clientFD.fd, clientData->writeContext.c_str(), clientData->writeContext.size(), 0);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        res = 0;
    }
    
    return res;
}

bool clearClientContext(std::shared_ptr<clientContext> clientData){
    try
    {
        clientData->readArguments.clear();
        clientData->writeBeforeRESP.clear();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    return 1;
}