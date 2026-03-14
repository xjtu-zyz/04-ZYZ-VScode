import socket
import threading
import datetime

def handle_client(client_socket, client_address):
    """
    处理客户端连接的线程函数
    记录客户端IP、端口和消息，并原样返回
    """
    print(f"\n[+] [{datetime.datetime.now().strftime('%H:%M:%S')}] 新客户端接入: {client_address}")
    print(f"    来自 {client_address[0]}:{client_address[1]} 的连接已建立")

    try:
        while True:
            # 接收客户端数据，缓冲区大小4096字节
            data = client_socket.recv(4096)

            if not data:
                # 客户端关闭连接
                print(f"[-] [{datetime.datetime.now().strftime('%H:%M:%S')}] 客户端 {client_address} 主动断开连接")
                break

            # 解码接收到的数据
            message = data.decode('utf-8')

            # 按照要求格式输出：from ('IP', port): 消息内容
            log_message = f"from {client_address}: {message}"
            print(f"    [接收] {log_message}")

            # 构造响应消息（回声）
            response = f"Server received: {message}"

            # 发送响应给客户端
            try:
                client_socket.send(response.encode('utf-8'))
                print(f"    [发送] 响应已发送给 {client_address}")
            except socket.error as e:
                print(f"[!] 向 {client_address} 发送响应失败: {e}")
                break

    except ConnectionResetError:
        print(f"[-] [{datetime.datetime.now().strftime('%H:%M:%S')}] 客户端 {client_address} 强制断开连接")
    except socket.error as e:
        print(f"[!] 与客户端 {client_address} 通信出错: {e}")
    finally:
        # 关闭客户端套接字
        client_socket.close()
        print(f"[-] [{datetime.datetime.now().strftime('%H:%M:%S')}] 客户端 {client_address} 连接已清理")
        print(f"[*] 当前活跃连接数: {threading.active_count() - 2}\n")

def tcp_server():
    """
    TCP服务器主函数
    监听0.0.0.0:8888，支持多客户端并发
    """
    # 服务器配置
    host = '0.0.0.0'  # 监听所有网络接口（包括localhost、内网IP、外网IP）
    port = 8888       # 监听端口

    # 创建TCP套接字
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # 设置套接字选项：允许地址复用，避免"地址已被使用"错误
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        # 绑定地址和端口
        server_socket.bind((host, port))
        print(f"[*] [{datetime.datetime.now().strftime('%H:%M:%S')}] TCP服务器启动成功")
        print(f"[*] 监听地址: {host}:{port}")
        print(f"[*] 本地访问: localhost:{port}")

        # 获取本机IP供外部访问
        try:
            import socket as sk
            s = sk.socket(sk.AF_INET, sk.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
            print(f"[*] 局域网访问: {local_ip}:{port}")
        except:
            pass

        print(f"[*] 等待客户端连接... (按Ctrl+C停止服务器)\n")

        # 开始监听，backlog=5表示最多5个排队连接
        server_socket.listen(5)

        while True:
            # 接受新连接（阻塞式）
            client_socket, client_address = server_socket.accept()

            # 为每个客户端创建独立线程
            client_thread = threading.Thread(
                target=handle_client,
                args=(client_socket, client_address),
                daemon=True  # 守护线程，主程序退出时自动结束
            )
            client_thread.start()

            print(f"[*] 当前活跃连接数: {threading.active_count() - 1}")

    except KeyboardInterrupt:
        print(f"\n[*] [{datetime.datetime.now().strftime('%H:%M:%S')}] 接收到停止信号，服务器正在关闭...")
    except socket.error as e:
        print(f"[!] 服务器错误: {e}")
    finally:
        # 关闭服务器套接字
        server_socket.close()
        print(f"[*] [{datetime.datetime.now().strftime('%H:%M:%S')}] 服务器已关闭")

if __name__ == '__main__':
    tcp_server()