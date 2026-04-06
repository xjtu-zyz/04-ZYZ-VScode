import time
import threading
from typing import Dict, Optional, List, Tuple


class MacTableEntry:
    """MAC地址表项"""
    def __init__(self, port: int, timestamp: float):
        self.port = port
        self.timestamp = timestamp


class MacTable:
    """MAC地址表管理类"""

    def __init__(self, aging_time: int = 300):
        self.table: Dict[str, MacTableEntry] = {}
        self.aging_time = aging_time

    def learn(self, mac: str, port: int, current_time: float = None):
        """学习MAC地址，记录端口并更新时间戳"""
        if current_time is None:
            current_time = time.time()

        is_new = mac not in self.table
        self.table[mac] = MacTableEntry(port, current_time)

        if is_new:
            print(f"  [MAC学习] 新地址: {mac} -> 端口{port}")
        else:
            old_port = self.table[mac].port
            if old_port != port:
                print(f"  [MAC学习] 地址迁移: {mac} 端口{old_port} -> 端口{port}")

    def lookup(self, mac: str, current_time: float = None) -> Optional[int]:
        """查找MAC地址对应的端口，自动处理老化"""
        if current_time is None:
            current_time = time.time()

        entry = self.table.get(mac)
        if entry:
            # 检查是否过期
            if current_time - entry.timestamp < self.aging_time:
                return entry.port
            else:
                # 过期则删除
                del self.table[mac]
                print(f"  [MAC老化] {mac} 已超时，从表中删除")
        return None

    def aging_check(self):
        """主动检查并清理过期表项"""
        current_time = time.time()
        expired_macs = [
            mac for mac, entry in self.table.items()
            if current_time - entry.timestamp >= self.aging_time
        ]
        for mac in expired_macs:
            del self.table[mac]
            print(f"  [MAC老化] {mac} 已超时，从表中删除")
        return len(expired_macs)

    def show_table(self):
        """显示MAC地址表"""
        if not self.table:
            print("  MAC地址表为空")
            return

        current_time = time.time()
        print("\n当前MAC地址表:")
        print(f"{'MAC地址':<20} {'端口':<6} {'剩余时间(秒)':<12}")
        print("-" * 40)
        for mac, entry in self.table.items():
            remaining = self.aging_time - (current_time - entry.timestamp)
            print(f"{mac:<20} {entry.port:<6} {remaining:.1f}")
        print(f"表项总数: {len(self.table)}")


class Switch:
    """二层交换机"""

    def __init__(self, port_count: int = 8, aging_time: int = 300):
        """
        交换机初始化
        :param port_count: 端口数量，默认8个
        :param aging_time: 老化时间，默认300秒
        """
        self.port_count = port_count
        self.mac_table = MacTable(aging_time)
        self.broadcast_count = 0  # 记录广播报文数量

        # 统计信息
        self.stats = {
            'total_frames': 0,
            'unicast_known': 0,
            'unicast_unknown': 0,
            'discarded': 0
        }

        print(f"交换机初始化完成:")
        print(f"  - 端口数量: {port_count}")
        print(f"  - 老化时间: {aging_time}秒")
        print(f"  - 端口列表: 1-{port_count}")
        print()

    def receive_frame(self, src_mac: str, dst_mac: str, ingress_port: int) -> List[int]:
        """
        接收帧并处理
        :param src_mac: 源MAC地址
        :param dst_mac: 目的MAC地址
        :param ingress_port: 接收端口号 (1-based)
        :return: 转发端口列表
        """
        # 验证端口范围
        if ingress_port < 1 or ingress_port > self.port_count:
            print(f"  [错误] 无效端口: {ingress_port}")
            return []

        self.stats['total_frames'] += 1
        current_time = time.time()

        print(f"\n[接收帧] 端口{ingress_port} <- 源:{src_mac} 目的:{dst_mac}")

        # 1. MAC地址学习：从接收到的帧中提取源MAC地址
        self.mac_table.learn(src_mac, ingress_port, current_time)

        # 2. 判断帧类型并转发
        # 广播报文转发：识别目的MAC为FF:FF:FF:FF:FF:FF的帧
        if dst_mac == "FF:FF:FF:FF:FF:FF":
            self.broadcast_count += 1
            print(f"  [广播帧] 目的MAC为广播地址")
            out_ports = self._flood(ingress_port)
            print(f"  [转发] 从所有其他端口转发: {out_ports}")
            return out_ports

        # 单播报文转发
        egress_port = self.mac_table.lookup(dst_mac, current_time)

        if egress_port is not None:
            # 已知单播：目的MAC在MAC表中
            self.stats['unicast_known'] += 1

            if egress_port != ingress_port:
                # 目的端口 ≠ 接收端口：精准转发
                print(f"  [已知单播] 目的MAC在端口{egress_port}")
                print(f"  [转发] 精准转发到端口: [{egress_port}]")
                return [egress_port]
            else:
                # 目的端口 = 接收端口：丢弃
                self.stats['discarded'] += 1
                print(f"  [丢弃] 目的端口{egress_port}等于接收端口，丢弃")
                return []
        else:
            # 未知单播：目的MAC不在MAC表中，当作广播处理
            self.stats['unicast_unknown'] += 1
            print(f"  [未知单播] 目的MAC不在MAC表中")
            out_ports = self._flood(ingress_port)
            print(f"  [转发] 泛洪到所有其他端口: {out_ports}")
            return out_ports

    def _flood(self, ingress_port: int) -> List[int]:
        """
        泛洪：从所有其他端口转发
        :param ingress_port: 接收端口
        :return: 转发端口列表
        """
        return [port for port in range(1, self.port_count + 1) if port != ingress_port]

    def show_mac_table(self):
        """显示MAC地址表"""
        print(f"\n=== 交换机MAC地址表 ===")
        self.mac_table.show_table()
        print(f"\n广播报文总数: {self.broadcast_count}")

    def show_stats(self):
        """显示统计信息"""
        print("\n=== 交换机统计信息 ===")
        print(f"总接收帧数: {self.stats['total_frames']}")
        print(f"广播帧数: {self.broadcast_count}")
        print(f"已知单播数: {self.stats['unicast_known']}")
        print(f"未知单播数: {self.stats['unicast_unknown']}")
        print(f"丢弃帧数: {self.stats['discarded']}")


# ===================== 测试函数 =====================
def test_switch():
    """测试交换机功能"""
    print("=" * 60)
    print("二层交换机转发机制模拟程序")
    print("=" * 60)

    # 初始化交换机：创建8个端口，老化时间300秒
    sw = Switch(port_count=8, aging_time=300)

    # 测试1: 未知单播 - 学习MAC并泛洪
    print("\n【测试1：未知单播 - 泛洪】")
    print("-" * 40)
    ports1 = sw.receive_frame("00:11:22:33:44:11", "AA:BB:CC:DD:EE:FF", 1)
    print(f"转发结果: {ports1}")

    # 测试2: 广播报文转发
    print("\n【测试2：广播报文转发】")
    print("-" * 40)
    ports2 = sw.receive_frame("00:11:22:33:44:22", "FF:FF:FF:FF:FF:FF", 2)
    print(f"转发结果: {ports2}")

    # 测试3: 已知单播 - 精准转发
    print("\n【测试3：已知单播 - 精准转发】")
    print("-" * 40)
    ports3 = sw.receive_frame("00:11:22:33:44:33", "00:11:22:33:44:11", 3)
    print(f"转发结果: {ports3}")

    # 测试4: 已知单播 - 同端口丢弃
    print("\n【测试4：已知单播 - 同端口丢弃】")
    print("-" * 40)
    ports4 = sw.receive_frame("00:11:22:33:44:44", "00:11:22:33:44:11", 1)
    print(f"转发结果: {ports4}")

    # 测试5: 学习第二个MAC地址
    print("\n【测试5：学习新MAC地址】")
    print("-" * 40)
    ports5 = sw.receive_frame("00:11:22:33:44:55", "00:11:22:33:44:66", 5)
    print(f"转发结果: {ports5}")

    # 显示MAC地址表
    sw.show_mac_table()

    # 测试6: MAC地址更新（端口迁移）
    print("\n【测试6：MAC地址端口迁移】")
    print("-" * 40)
    ports6 = sw.receive_frame("00:11:22:33:44:11", "00:11:22:33:44:55", 7)
    print(f"转发结果: {ports6}")

    # 显示更新后的MAC地址表
    sw.show_mac_table()

    # 显示统计信息
    sw.show_stats()

    print("\n【注意】由于老化时间为300秒，所有MAC地址在测试期间都不会老化")
    print("MAC地址表将保持所有学习到的地址")


if __name__ == "__main__":
    test_switch()