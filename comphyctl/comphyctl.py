import sys
import socket
import os

SOCKET_PATH = "/tmp/comphy/ctl"


def main():
    arguments = sys.argv[1:]
    formatted = " ".join([f'"{arg}"' for arg in arguments])

    if not os.path.exists(SOCKET_PATH):
        print("comphyctl socket not found")
        return

    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.connect(SOCKET_PATH)
            s.sendall(formatted.encode("utf-8"))

    except Exception as e:
        print(f"there was an error: {e}")


if __name__ == "__main__":
    main()
