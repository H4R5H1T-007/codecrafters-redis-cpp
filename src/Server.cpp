#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <future>
#include <queue>
#include <mutex>
#include <poll.h>

std::queue<int> client_queue;
std::mutex client_queue_lock;

int asyncAccept(int, sockaddr *, socklen_t *);
void asyncResponse(int fd);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!!\n";

  // Uncomment this block to pass the first stage
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    perror("bind failed. Error");
    return 1;
  }
  
  int connection_backlog = 1000;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  // auto async_accept = std::async(asyncAccept, server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::vector<pollfd> fds;
  int new_socket;
  fds.push_back({server_fd, POLLIN, 0});
  int count = 0;

  while (true) {
      int activity = poll(fds.data(), fds.size(), -1);
      if (activity < 0) {
          std::cerr << "Poll error" << std::endl;
          perror("poll failed. Error");
          break;
      }
      std::cout<<"Poll activated";
      // if(count%1000 == 0){
      //   std::cout<<"16k reached.\n";
      // }

      std::vector<std::future<void>> task_list;

      for (int i = 0; i < fds.size(); ) {
          std::cout<<"file desc is "<<fds[i].fd<<" POLLIn is "<<POLLIN<<" server_fd is "<<server_fd<<"\n";
          if ((fds[i]).revents & POLLIN && fds[i].fd != -1) {
              if (fds[i].fd == server_fd) {
                  // New connection
                  std::cout<<"It came inside this fd\n";
                  new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
                  std::cout<<"new socket is "<<new_socket<<"\n";
                  if (new_socket < 0) {
                      std::cerr << "Accept failed" << std::endl;
                      continue;
                  }
                  count++;
                  // std::cout<<(*it).fd<<" "<<new_socket<<std::endl;

                  fds.push_back({new_socket, POLLIN, 0});
              } else {
                
                task_list.push_back(std::async(asyncResponse, fds[i].fd));
                fds[i].fd = -1;
                  // Data from client
                  // char buffer[1024] = {0};
                  // int valread = read(fds[i].fd, buffer, 1024);
                  // if (valread <= 0) {
                  //     // Connection closed
                  //     close(fds[i].fd);
                  //     // it = fds.erase(it);
                  //     fds[i].fd = -1;
                  //     continue;
                  // }

                  // // std::cout << "Received: " << buffer << std::endl;
                  // // Echo back
                  // const char* message = "+PONG\r\n";
                  // send(fds[i].fd, message, strlen(message), 0);
              }
          }
          ++i;
      }

      std::vector<pollfd> temp;
      // temp.push_back({server_fd, POLLIN, 0});
      for(auto it = fds.begin(); it != fds.end(); it++){
        if((*it).fd != -1)
          temp.push_back(*it);
      }
      fds = temp;
  }
  
  // Passing accepting connections to different thread so main thread (and inturn event loop) remains unblocked

  // while(true){
  //   // struct sockaddr_in client_addr;
  //   // int client_addr_len = sizeof(client_addr);
  //   // int clientSocket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  //   int clientSocket = -1;
  //   {
  //     // std::lock_guard<std::mutex> queue_lock(client_queue_lock);
  //     if(!client_queue.empty()){
  //       clientSocket = client_queue.front();
  //       client_queue.pop();
  //     }
  //   }
  //   if(clientSocket < 0){
  //     continue;
  //   }
  //   std::cout<<"Main Thread"<<clientSocket<<std::endl;
  //   std::cout << "Client connected\n";
  //   const char* message = "+PONG\r\n";
  //   char buffer[128];
  //   while(recv(clientSocket, buffer, 127, 0) > 0){
  //     int n = write(clientSocket, message, strlen(message));
  //   }

  // }
  
  close(server_fd);

  return 0;
}

void asyncResponse(int fd){
  char buffer[1024];
  const char* message = "+PONG\r\n";
  while(recv(fd, buffer, 1023, 0) > 0){
    int n = write(fd, message, strlen(message));
  }
  // Connection closed
  close(fd);
  // it = fds.erase(it);
}

int asyncAccept(int server_fd, sockaddr *, socklen_t *){
  while(true){
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    int clientSocket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    {
      std::cout<<"Async Thread\n"<<clientSocket;
      std::lock_guard<std::mutex> queue_lock(client_queue_lock);
      client_queue.push(clientSocket);
    }
  }
}