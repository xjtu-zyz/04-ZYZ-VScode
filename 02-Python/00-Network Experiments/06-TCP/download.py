import socket
import time
import sys

def download_file(host, port, path, save_as):
    """
    通过 HTTP/1.1 GET 下载文件并保存到本地
    返回：下载耗时（毫秒）
    """
    print(f"开始下载: http://{host}:{port}{path}")
    start_time = time.time()

    # 1. 创建 TCP 连接
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(30)
    sock.connect((host, port))

    # 2. 构造 HTTP/1.1 GET 请求
    request = (
        f"GET {path} HTTP/1.1\r\n"
        f"Host: {host}\r\n"
        f"Connection: close\r\n"
        f"User-Agent: TCP-Lab-Client\r\n"
        f"\r\n"
    )
    sock.sendall(request.encode())

    # 3. 接收响应（先读响应头找到空行，再接收剩余的文件数据）
    response_data = b""
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        response_data += chunk
    sock.close()

    # 4. 分离响应头和文件内容
    # 响应头与数据之间以 b'\r\n\r\n' 分隔
    header_end = response_data.find(b'\r\n\r\n')
    if header_end == -1:
        print("错误：未找到响应头结束标志")
        return -1
    file_content = response_data[header_end + 4:]

    # 5. 写入本地文件
    with open(save_as, 'wb') as f:
        f.write(file_content)

    # 6. 计算耗时
    end_time = time.time()
    elapsed_ms = (end_time - start_time) * 1000

    print(f"文件已保存为: {save_as}")
    print(f"下载耗时: {elapsed_ms:.2f} ms")
    return elapsed_ms


if __name__ == "__main__":
    # 默认参数（实验六云服务器地址和端口）
    HOST = "你的云服务器公网IP"      # 例如 "123.123.123.123"
    PORT = 8080                     # Python HTTP 服务器端口
    PATH = "/root/Project_6"          # 要下载的文件路径
    SAVE_AS = "/m25.4.zip"  # 本地保存文件名

    # 支持命令行覆盖参数
    if len(sys.argv) >= 2:
        HOST = sys.argv[1]
    if len(sys.argv) >= 3:
        PORT = int(sys.argv[2])
    if len(sys.argv) >= 4:
        PATH = sys.argv[3]
    if len(sys.argv) >= 5:
        SAVE_AS = sys.argv[4]

    download_file(HOST, PORT, PATH, SAVE_AS)