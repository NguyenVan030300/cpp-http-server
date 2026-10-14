#pragma once
#include <netinet/in.h>

#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>

class TcpServer {
 public:
  // Constructor initialize the socket, binds, and listen
  explicit TcpServer(uint16_t port);
  ~TcpServer();

  void start();

 private:
  uint16_t port_;
  int server_fd_;
  std::mutex mutex_;

  void safetyLog(const std::string& message);
  void handleClient(int client_fd, sockaddr_in client_addr);
  void runSnakeGame(int client_fd);
  void runTetrisGame(int client_fd);
};
