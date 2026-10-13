# High-Performance Multi-Threaded C++ HTTP Server

A modular, production-ready HTTP/1.1 web server built entirely from scratch in modern C++ (C++17) using low-level POSIX socket APIs.

## Key Engineering Features
* **RAII Resource Management:** Automated file descriptor handling and socket lifecycle safety in destructors (`SO_REUSEADDR`).
* **Concurrency Model:** Thread-per-connection architecture leveraging `std::thread` and thread-safe logging guarded by `std::mutex` and `std::lock_guard`.
* **HTTP/1.1 Protocol Engine:** Minimalist request parser and routing engine capable of handling `GET` requests and returning proper `200 OK` and `404 Not Found` responses with dynamic content lengths.
* **Robust Debugging & Verification:** Tested for memory safety using Valgrind Memcheck and verified via post-mortem Linux core dumps and GDB stack inspections.

## Getting Started

### Prerequisites
* Linux (Ubuntu / WSL)
* CMake (>= 3.15)
* A C++17 compatible compiler (GCC / Clang)

### Build Instructions
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .