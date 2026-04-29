import socket

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', 9000))
    server_socket.listen(5)
    print("Server started, waiting for client...")

    while True:
        client_socket, client_addr = server_socket.accept()

        # 客户端公网IP（正确）
        print("Client public address:", client_addr)

        # 服务器真实私网IP（关键在这里！）
        print("Server private address:", client_socket.getsockname())

        try:
            file_name = client_socket.recv(1024).decode('utf-8')
            print("Client requests file:", file_name)

            with open(file_name, 'rb') as f:
                data = f.read()
                client_socket.sendall(data)

            print("File sent\n")

        except Exception as e:
            print("Error:", e)
        finally:
            client_socket.close()

if __name__ == "__main__":
    main()