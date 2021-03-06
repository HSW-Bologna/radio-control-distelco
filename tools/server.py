import socket

HOST = '0.0.0.0'  # Standard loopback interface address (localhost)
PORT = 8082        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()

    while True:
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            while True:
                try:
                    data = conn.recv(1024)
                    print(f'received {data}')
                    if not data:
                        continue
                    conn.sendall(
                        bytes([1]*18) + bytes(b'Simulatore PC  ') + bytes([0]) + bytes([1]*70))
                except (KeyboardInterrupt, ConnectionResetError):
                    conn.close()
                    break
