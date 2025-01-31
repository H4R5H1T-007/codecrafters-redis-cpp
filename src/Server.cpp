#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include "networking.hpp"
#include "eventLoop.hpp"
#include "commands.hpp"

void pollAfterSomeTime(){
    pollServer();
    el.timeSubmit(pollAfterSomeTime, 100);
}

void pollContiniously(){
    pollServer();
    el.fileSubmit(pollContiniously);
}

int main(int argc, char **argv) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here!!\n";

    if(!initServerConnection()){
       return 1; 
    }

    if(!cmdTableCreator()){
        return 1;
    }

    el.fileSubmit(pollContiniously);
    el.run();


    std::cerr<<"poll error!"<<std::endl;
  // Uncomment this block to pass the first stage
  
//   int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//   if (server_fd < 0) {
//    std::cerr << "Failed to create server socket\n";
//    return 1;
//   }
  
//   // Since the tester restarts your program quite often, setting SO_REUSEADDR
//   // ensures that we don't run into 'Address already in use' errors
//   int reuse = 1;
//   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
//     std::cerr << "setsockopt failed\n";
//     return 1;
//   }
  
//   struct sockaddr_in server_addr;
//   server_addr.sin_family = AF_INET;
//   server_addr.sin_addr.s_addr = INADDR_ANY;
//   server_addr.sin_port = htons(6379);
  
//   if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
//     std::cerr << "Failed to bind to port 637\n";
//     perror("bind failed. Error");
//     return 1;
//   }
  
//   int connection_backlog = 1000;
//   if (listen(server_fd, connection_backlog) != 0) {
//     std::cerr << "listen failed\n";
//     return 1;
//   }
  

//   struct sockaddr_in client_addr;
//   int client_addr_len = sizeof(client_addr);
//   // auto async_accept = std::async(asyncAccept, server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
//     std::vector<pollfd> fds;
//     fds.push_back({server_fd, POLLIN, 0});
//     int count = 0;

    // while (true) {
    //     // std::cout << "Entering poll() with " << fds.size() << " fds\n";
    //     int activity = poll(fds.data(), fds.size(), -1);
        
    //     if (activity < 0) {
    //         std::cerr << "Poll error: " << strerror(errno) << std::endl;
    //         break;
    //     }
        
    //     // std::cout << "Activity: " << activity << ", Poll activated\n";

    //     for (size_t i = 0; i < fds.size(); ++i) {
    //         // std::cout << "Checking fd: " << fds[i].fd << ", revents: " << fds[i].revents << std::endl;
            
    //         if (fds[i].revents & POLLIN) {
    //             if (fds[i].fd == server_fd) {
    //                 // New connection
    //                 // std::cout << "New connection incoming\n";
    //                 int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
                    
    //                 if (new_socket < 0) {
    //                     std::cerr << "Accept failed: " << strerror(errno) << std::endl;
    //                 } else {
    //                     // std::cout << "New socket accepted: " << new_socket << std::endl;
    //                     fds.push_back({new_socket, POLLIN, 0});
    //                     count++;
    //                 }
    //             } else {
    //                 // Existing connection has data
    //                 // std::cout << "Data received on fd: " << fds[i].fd << std::endl;
    //                 char buffer[1024] = {0};
    //                 int valread = read(fds[i].fd, buffer, sizeof(buffer));
    //                 if (valread > 0) {
    //                     // std::cout << "Received " << valread << " bytes: " << buffer << std::endl;
    //                     // printf(buffer);
    //                     // printf("\n");
    //                     // fwrite(buffer, 1, strlen(buffer), stdout);
    //                     // if buffer starts with a * that means it's an array

    //                     // auto res = parseClientInput(buffer, valread);
                        
                        
    //                     char* message = "+PONG\r\n";
    //                     int sent = send(fds[i].fd, message, strlen(message), 0);
    //                     // std::cout << "Sent " << sent << " bytes: " << message << std::endl;
    //                 } else if (valread == 0) {
    //                     // std::cout << "Client disconnected: " << fds[i].fd << std::endl;
    //                     close(fds[i].fd);
    //                     fds.erase(fds.begin() + i);
    //                     --i;
    //                 } else {
    //                     // std::cerr << "Read error on socket " << fds[i].fd << ": " << strerror(errno) << std::endl;
    //                     close(fds[i].fd);
    //                     fds.erase(fds.begin() + i);
    //                     --i;
    //                 }
    //             }
    //         } else if (fds[i].revents & (POLLHUP | POLLERR)) {
    //             // Connection closed or error
    //             // std::cout << "Connection closed or error on fd: " << fds[i].fd << std::endl;
    //             close(fds[i].fd);
    //             fds.erase(fds.begin() + i);
    //             --i;  // Adjust index after erasing
    //         }
    //     }
    // }

    // // Cleanup
    // for (const auto& fd : fds) {
    //     close(fd.fd);
    // }

    return 0;
}

