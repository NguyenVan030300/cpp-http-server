#include <exception>
#include <iostream>

#include "httpsProject.hpp"

int main() {
  try {
    TcpServer server(8080);
    server.start();
  } catch (const std::exception& e) {
    std::cerr << "Server Crashed" << e.what() << "\n";
    return 1;
  }
  return 0;
}