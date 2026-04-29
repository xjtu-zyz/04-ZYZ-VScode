# tcp_server.py
import socket

def main():
    # 1. 创建TCP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # 2. 绑定地址和端口 (0.0.0.0表示监听所有网卡接口)
    server_addr = ('0.0.0.0', 9000)
    server_socket.bind(server_addr)

    # 3. 开始监听，最大等待连接数设为5
    server_socket.listen(5)
    print("服务器启动，等待客户端连接...")

    while True:
        # 4. 等待客户端连接
        client_socket, client_addr = server_socket.accept()
        print(f"客户端 {client_addr} 已连接")

        try:
            # 5. 接收客户端请求的文件名
            file_name = client_socket.recv(1024).decode('utf-8')
            print(f"客户端请求文件: {file_name}")

            # 6. 打开文件并读取内容
            with open(file_name, 'rb') as f:
                file_data = f.read()

            # 7. 发送文件内容给客户端
            client_socket.sendall(file_data)
            print("文件发送完成")

        except FileNotFoundError:
            print("文件不存在")
        except Exception as e:
            print(f"发生错误: {e}")
        finally:
            # 8. 关闭与客户端的连接
            client_socket.close()

if __name__ == "__main__":
    main()