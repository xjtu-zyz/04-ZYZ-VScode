from scapy.all import ARP, Ether, srp, conf
import socket
import platform

def get_local_info():
    """获取本机网络信息，针对WLAN 10.183.200.0/24网段优化"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()

        # 匹配WLAN网段 10.183.200.x/24
        if local_ip.startswith("10.183.200."):
            return {
                'ip': local_ip,
                'subnet': "10.183.200.0/24",
                'gateway': "10.183.200.156",
                'interface': "WLAN"
            }
        else:
            return {
                'ip': "10.183.200.225",
                'subnet': "10.183.200.0/24",
                'gateway': "10.183.200.156",
                'interface': "WLAN (手动)"
            }
    except:
        return {
            'ip': "10.183.200.225",
            'subnet': "10.183.200.0/24",
            'gateway': "10.183.200.156",
            'interface': "WLAN (手动)"
        }

def scan_network(network=None, timeout=3):
    """扫描局域网主机"""
    local_info = get_local_info()

    if network is None:
        network = local_info['subnet']

    print(f"本机IP: {local_info['ip']}")
    print(f"扫描网段: {network}")
    print(f"默认网关: {local_info['gateway']}")
    print("发送ARP探测包...\n")

    if platform.system() == "Windows":
        conf.verb = 0

    try:
        # 构造ARP广播包
        arp_request = ARP(pdst=network)
        broadcast = Ether(dst="ff:ff:ff:ff:ff:ff")
        arp_packet = broadcast / arp_request

        # 发送并接收响应
        answered, _ = srp(arp_packet, timeout=timeout, verbose=False)

        devices = []
        for sent, received in answered:
            ip_addr = received.psrc
            mac_addr = received.hwsrc

            # 标记本机和网关
            marker = ""
            if ip_addr == local_info['ip']:
                marker = "[本机]"
            elif ip_addr == local_info['gateway']:
                marker = "[网关]"

            devices.append({
                'ip': ip_addr,
                'mac': mac_addr,
                'marker': marker
            })

        # 按IP排序
        devices.sort(key=lambda x: [int(i) for i in x['ip'].split('.')])

        # 输出结果
        print(f"{'标记':<8} {'IP地址':<18} {'MAC地址':<20}")
        print("-" * 50)
        for device in devices:
            print(f"{device['marker']:<8} {device['ip']:<18} {device['mac']:<20}")

        print(f"\n[*] 发现 {len(devices)} 台活跃主机")
        return devices

    except Exception as e:
        print(f"[!] 扫描出错: {e}")
        return []

if __name__ == '__main__':
    scan_network()