#include "httpsProject.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

constexpr int       BACKLOG = 10;
constexpr size_t    BUFFER_SIZE = 1024;

void TcpServer::safetyLog(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << message << std::endl;
}

TcpServer::TcpServer(uint16_t port) : port_(port), server_fd_(-1)
{
    // AF_INET = IPv4
    // SOCK_STREAM = TCP
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0 ) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    }

    sockaddr_in address       {};
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(port_);

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to bind to port");
    }

    if (listen(server_fd_, BACKLOG) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to listen on socket");
    }

    safetyLog("[Server] Initialized and bound to port" + std::to_string(port_));
}

TcpServer::~TcpServer()
{
    if (server_fd_ >= 0) {
        close(server_fd_);
        safetyLog("[Server] Socket closed gracefully.");
    }
}

void TcpServer::start()
{
    safetyLog("[Server] Waiting for client connections ...");

    while (true) {
        sockaddr_in client_addr   {};
        socklen_t   addrlen     = sizeof(client_addr);

        // Waiting for connection in mainThread
        int client_fd = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &addrlen);
        if (client_fd < 0) {
            std::cerr << "[Error] Failed to accept client connection \n";
            continue;
        }

        //Passing handleClient to another thread to handle
        std::thread worker(&TcpServer::handleClient, this, client_fd, client_addr);

        // Detach allows the thread to run independently
        worker.detach();
    }
}

void TcpServer::handleClient(int client_fd, sockaddr_in client_addr)
{
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    safetyLog(std::string("[Server] Client connected from ") + client_ip + " : " + std::to_string(ntohs(client_addr.sin_port)));

    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, sizeof(buffer));
    
    // Read the incoming HTTP request from the browser/curl
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        std::string request(buffer);
        
        // Safely extract just the first line of the HTTP request for logging
        size_t first_line_end = request.find("\n");
        std::string first_line = (first_line_end != std::string::npos) ? request.substr(0, first_line_end) : request;
        safetyLog("[HTTP Request Received]: " + first_line);

        std::string response_body;
        std::string status_line;

        // Simple routing check
        if (request.find("GET /index.html") != std::string::npos || request.find("GET / ") != std::string::npos) {
            status_line = "HTTP/1.1 200 OK\r\n";
            response_body = 
                "<html><head><title>C++ Custom Server</title></head>"
                "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>"
                "<h1>Hello from Custom C++ Multi-Threaded HTTP Server!</h1>"
                "<p>Sprint 4: HTTP/1.1 Protocol Routing Successful.</p>"
                "</body></html>";
        } else {
            status_line = "HTTP/1.1 404 Not Found\r\n";
            response_body = "<html><body><h1>404 Page Not Found</h1></body></html>";
        }

        // Construct standard HTTP/1.1 response packet with Connection: close
        std::string http_response = 
            status_line +
            "Content-Type: text/html; charset=UTF-8\r\n" +
            "Content-Length: " + std::to_string(response_body.size()) + "\r\n" +
            "Connection: close\r\n\r\n" +
            response_body;

        // Send response back to the client
        write(client_fd, http_response.c_str(), http_response.size());
        safetyLog("[Server] HTTP response sent.");

    } else if (bytes_read == 0) {
        safetyLog("[Server] Client closed connection prematurely.");
    } else {
        safetyLog("[Error] Read failed on client socket.");
    }

    // Crucial: Close the socket and let the worker thread terminate cleanly
    close(client_fd);
    safetyLog("[Server] Client socket closed.");
}
