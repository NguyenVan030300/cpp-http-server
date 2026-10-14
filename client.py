import socket
import sys
import termios
import tty
import select

HOST = '127.0.0.1'
PORT = 8080

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((HOST, PORT))
    except ConnectionRefusedError:
        print("[Error] Could not connect to C++ server. Is it running?")
        return

    # Save original terminal settings
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)

    try:
        # Set terminal to raw mode (intercepts keys instantly without Enter)
        tty.setraw(fd)
        
        print("Connected to C++ Arcade! Use WASD to play. Press Ctrl+C to exit.\r")

        while True:
            # Multiplex between socket input (server frames) and stdin (player keys)
            rlist, _, _ = select.select([s, sys.stdin], [], [], 0.05)

            if s in rlist:
                data = s.recv(4096)
                if not data:
                    break
                # Print server screen buffer directly to player's terminal
                sys.stdout.write(data.decode('utf-8', errors='ignore'))
                sys.stdout.flush()

            if sys.stdin in rlist:
                ch = sys.stdin.read(1)
                if ch == '\x03':  # Ctrl+C
                    break
                # Send the single key instantly to the C++ server
                s.sendall(ch.encode('utf-8'))

    finally:
        # Restore original terminal settings upon exit
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        s.close()
        print("\r\nDisconnected from arcade.\r\n")

if __name__ == '__main__':
    main()