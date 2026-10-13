#include "httpsProject.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <deque>
#include <random>
#include <sys/time.h>

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

void TcpServer::runTetrisGame(int client_fd) {
    // 1. Configure socket read timeout for game tick rate (150ms)
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 150000; // 150 milliseconds per frame tick
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

void TcpServer::runSnakeGame(int client_fd) {

    // 1. Configure socket read timeout for game tick rate (150ms)
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 150000; // 150 milliseconds per frame tick
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    // Welcome banner
    std::string welcome = "\033[2J\033[H=== C++ TCP SNAKE ARCADE ===\r\nControls: W (Up), A (Left), S (Down), D (Right) + Enter\r\nStarting game...\r\n";
    write(client_fd, welcome.c_str(), welcome.size());

    // Game Configuration
    const int width = 20;
    const int height = 10;
    std::deque<std::pair<int, int>> snake = {{10, 5}, {9, 5}, {8, 5}};
    int dx = 1;
    int dy = 0;
    int food_x = 5;
    int food_y = 5;
    bool game_over = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis_x(1, width - 2);
    std::uniform_int_distribution<> dis_y(1, height - 2);

    // Main Game Loop
    while (!game_over) {
        // Read non-blocking input from client socket
        char buf[16] = {0};
        ssize_t bytes_read = read(client_fd, buf, sizeof(buf) - 1);
        if (bytes_read > 0) {
            char cmd = buf[0];
            if (cmd == 'w' && dy == 0) { dx = 0; dy = -1; }
            else if (cmd == 's' && dy == 0) { dx = 0; dy = 1; }
            else if (cmd == 'a' && dx == 0) { dx = -1; dy = 0; }
            else if (cmd == 'd' && dx == 0) { dx = 1; dy = 0; }
            else if (cmd == 'q') { break; }
        }

        // Update Snake Position
        std::pair<int, int> head = snake.front();
        std::pair<int, int> new_head = {head.first + dx, head.second + dy};

        // Check Wall Collision
        if (new_head.first <= 0 || new_head.first >= width - 1 || new_head.second <= 0 || new_head.second >= height - 1) {
            game_over = true;
            break;
        }

        // Check Self Collision
        for (const auto& seg : snake) {
            if (seg == new_head) {
                game_over = true;
                break;
            }
        }
        if (game_over) break;

        snake.push_front(new_head);

        // Check Food Collision
        if (new_head.first == food_x && new_head.second == food_y) {
            // Generate new food ensuring it doesn't land on the snake
            bool collision = false;
            do {
                food_x = dis_x(gen);
                food_y = dis_y(gen);
                collision = false;
                
                // Check against every segment of the snake
                for (const auto& seg : snake) {
                    if (seg.first == food_x && seg.second == food_y) {
                        collision = true; // Oops, hit the snake! Try again.
                        break;
                    }
                }
            } while (collision);
        } else {
            snake.pop_back(); // Remove tail if no food eaten
        }

        // Render Frame using ANSI Escape Sequences (`\033[H` moves cursor to top-left without flicker)
        std::string screen = "\033[H";
        screen += "=== C++ TCP SNAKE ARCADE ===\r\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    screen += "#"; // Walls
                } else if (x == food_x && y == food_y) {
                    screen += "*"; // Food
                } else {
                    bool is_body = false;
                    for (const auto& seg : snake) {
                        if (seg.first == x && seg.second == y) {
                            is_body = true;
                            break;
                        }
                    }
                    screen += (is_body ? "O" : " "); // Snake body or empty space
                }
            }
            screen += "\r\n";
        }
        screen += "Score: " + std::to_string((snake.size() - 3) * 10) + " | Press 'q' to quit\r\n";

        // Push frame over TCP socket to the client terminal
        ssize_t written = write(client_fd, screen.c_str(), screen.size());
        if (written <= 0) {
            break; // Client disconnected abruptly
        }
    }

    // Game Over Cleanup
    std::string game_over_msg = "\r\n\033[1;31mGAME OVER!\033[0m Final Score: " + std::to_string((snake.size() - 3) * 10) + "\r\nConnection closing.\r\n";
    write(client_fd, game_over_msg.c_str(), game_over_msg.size());
    close(client_fd);
    safetyLog("[Server] Snake session ended, client socket closed."); 
}

// (Keep your TcpServer constructor, destructor, and safetyLog methods as they are...)
void TcpServer::handleClient(int client_fd, sockaddr_in client_addr)
{
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    safetyLog(std::string("[Server] Client connected: ") + client_ip);

    // 1. Send the Arcade Lobby Menu
    std::string menu = 
        "\033[2J\033[H"
        "==============================\r\n"
        "    C++ MULTI-THREADED ARCADE   \r\n"
        "==============================\r\n"
        "  1. Snake Game\r\n"
        "  2. Tetris Game (Coming Next!)\r\n"
        "  q. Quit\r\n"
        "------------------------------\r\n"
        "Select an option (1-2): ";
    
    write(client_fd, menu.c_str(), menu.size());

    // 2. Read client choice
    char choice_buf[16] = {0};
    ssize_t bytes_read = read(client_fd, choice_buf, sizeof(choice_buf) - 1);
    
    if (bytes_read > 0) {
        char choice = choice_buf[0];
        
        if (choice == '1') {
            safetyLog("[Server] Client selected Snake.");
            runSnakeGame(client_fd);
        } 
        else if (choice == '2') {
            safetyLog("[Server] Client selected Tetris.");
            runTetrisGame(client_fd); // We will build this next!
        } 
        else {
            std::string goodbye = "Exiting arcade. Goodbye!\r\n";
            write(client_fd, goodbye.c_str(), goodbye.size());
        }
    }

    close(client_fd);
    safetyLog("[Server] Client session terminated, socket closed.");
}

/* HTTP website
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
*/