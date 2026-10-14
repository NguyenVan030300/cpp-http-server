# TCP Arcade Server

A C++17 TCP server that hosts a small terminal arcade. Clients connect to the
server, choose a game from a text menu, and receive terminal frames over the
socket. The server currently offers Snake and Tetris.

## How It Works

- `server_app` listens on port `8080` using POSIX TCP sockets.
- Each accepted client connection is handled by a separate worker thread.
- The server sends an ANSI terminal menu and streams the selected game's
	screen to the client.
- `client.py` connects to the server, forwards keyboard input, and displays
	the received terminal output.

The active application is a TCP arcade, not an HTTP server. An earlier HTTP
handler remains commented out in the source and is not part of the running
program.

## Requirements

- Linux or WSL
- CMake 3.14 or newer
- A C++17 compiler, such as GCC or Clang
- Python 3 for the terminal client

GoogleTest is fetched by CMake during configuration, so configuring the
project requires network access unless the dependency is already cached.

## Build and Run

From the project root, configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Start the server:

```bash
./build/server_app
```

In a second terminal, start the client:

```bash
python3 client.py
```

Choose a game from the menu in the client terminal. Press `Ctrl+C` to
disconnect; the client restores the terminal settings on exit.

## Tests

The `unit_tests` target uses GoogleTest. Build the project as above, then run:

```bash
./build/unit_tests
```