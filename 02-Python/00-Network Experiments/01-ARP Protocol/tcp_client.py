import socket
import sys

def tcp_client():
    # 服务器地址和端口
    server_host = 'localhost'  # 可以替换为服务器的IP地址
    server_port = 8888

    # 创建TCP套接字
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # 连接服务器
        print(f"[*] 正在连接服务器 {server_host}:{server_port}...")
        client_socket.connect((server_host, server_port))
        print("[+] 连接成功！")
        print("[*] 输入 'quit' 或 'exit' 可退出程序\n")

        # 通信循环，允许用户自主控制
        while True:
            # 用户输入消息
            try:
                message = input("[我] > ")
            except KeyboardInterrupt:
                print("\n[*] 检测到Ctrl+C，正在断开连接...")
                break

            # 检查退出命令
            if message.lower() in ['quit', 'exit', 'q']:
                print("[*] 正在断开连接...")
                break

            # 检查空消息
            if not message.strip():
                print("[!] 消息不能为空，请重新输入")
                continue

            # 发送消息到服务器
            try:
                client_socket.send(message.encode('utf-8'))
            except socket.error as e:
                print(f"[!] 发送失败: {e}")
                break

            # 接收服务器响应
            try:
                response = client_socket.recv(4096).decode('utf-8')
                if not response:
                    print("[!] 服务器已关闭连接")
                    break
                print(f"[服务器] < {response}\n")
            except socket.error as e:
                print(f"[!] 接收失败: {e}")
                break

        print("[*] 连接已关闭")

    except ConnectionRefusedError:
        print(f"[!] 无法连接到服务器 {server_host}:{server_port}")
        print("[!] 请检查服务器是否运行，或网络是否畅通")
    except TimeoutError:
        print("[!] 连接超时，请检查网络")
    except Exception as e:
        print(f"[!] 发生错误: {e}")
    finally:
        # 确保关闭套接字
        try:
            client_socket.close()
        except:
            pass
        print("[*] 客户端程序已退出")

if __name__ == '__main__':
    tcp_client()