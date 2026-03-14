# Blind Search
from collections import deque
import numpy as np

class TSP_BFS:
    def __init__(self, distance_matrix):
        self.distance_matrix = distance_matrix
        self.n = len(distance_matrix)
        self.best_path = None
        self.best_cost = float('inf')
        
    def solve(self, start_city=0):
        """使用BFS算法解决TSP问题"""
        # 队列元素: (current_city, visited, path, current_cost)
        # 使用位掩码表示visited状态，提高效率
        initial_state = (start_city, 1 << start_city, [start_city], 0)
        queue = deque([initial_state])
        
        while queue:
            current_city, visited, path, current_cost = queue.popleft()
            
            # 如果已经访问所有城市
            if bin(visited).count("1") == self.n:
                # 回到起点
                total_cost = current_cost + self.distance_matrix[current_city][start_city]
                if total_cost < self.best_cost:
                    self.best_cost = total_cost
                    self.best_path = path + [start_city]
                continue
            
            # 扩展当前节点的邻居
            for next_city in range(self.n):
                # 检查是否已访问
                if not (visited & (1 << next_city)):
                    new_visited = visited | (1 << next_city)
                    new_path = path + [next_city]
                    new_cost = current_cost + self.distance_matrix[current_city][next_city]
                    
                    # 剪枝：如果当前代价已经超过已知最优解，则不再加入队列
                    if new_cost < self.best_cost:
                        queue.append((next_city, new_visited, new_path, new_cost))
        
        return self.best_path, self.best_cost

def input_distance_matrix():
    """手动输入距离矩阵"""
    print("=== TSP问题距离矩阵输入 ===")
    
    # 输入城市数量
    while True:
        try:
            n = int(input("请输入城市数量: "))
            if n < 2:
                print("城市数量至少为2，请重新输入")
                continue
            break
        except ValueError:
            print("请输入有效的整数")
    
    # 初始化距离矩阵
    distance_matrix = [[0] * n for _ in range(n)]
    
    # 输入距离
    print("\n请输入城市之间的距离 (输入-1表示不可达):")
    for i in range(n):
        # 距离矩阵全部输入（易出错）
        # for j in range(n):
        #     while True:
        #         try:
        #             v =float(input(f"城市{i}到城市{j}的距离: "))
        #             distance_matrix[i][j] =v
        #             if v< 0:
        #                 print("距离不能为负数，请重新输入")
        #                 continue
        #             distance_matrix[i][j] =v
        #             break
        #         except ValueError:
        #             print("请输入有效的数字")
        for j in range(i+1, n):  # 只输入上三角部分，避免重复
            while True:
                try:
                    dist = input(f"城市{i}到城市{j}的距离: ")
                    if dist == "-1":
                        distance_matrix[i][j] = float('inf')
                        distance_matrix[j][i] = float('inf')
                    else:
                        dist_val = float(dist)
                        if dist_val < 0:
                            print("距离不能为负数，请重新输入")
                            continue
                        distance_matrix[i][j] = dist_val
                        distance_matrix[j][i] = dist_val
                    break
                except ValueError:
                    print("请输入有效的数字")
    return distance_matrix


# def main():
#     """主函数"""
#     print("TSP问题求解器 (使用BFS算法)")
    
#     distance_matrix = input_distance_matrix()

#     # 打印距离矩阵
#     print("\n距离矩阵:")
#     for i, row in enumerate(distance_matrix):
#         print(f"城市{i}: {row}")
    
#     # 选择起始城市
#     while True:
#         try:
#             start_city = int(input(f"\n请输入起始城市 (0-{len(distance_matrix)-1}): "))
#             if 0 <= start_city < len(distance_matrix):
#                 break
#             else:
#                 print(f"起始城市必须在 0-{len(distance_matrix)-1} 范围内")
#         except ValueError:
#             print("请输入有效的整数")
    
#     # 求解TSP问题
#     tsp_solver = TSP_BFS(distance_matrix)
#     path, cost = tsp_solver.solve(start_city)
    
#     # 输出结果
#     print("\n" )
#     print("计算结果:")
#     print(f"最优路径: {' -> '.join(map(str, path))}")
#     print(f"最短距离: {cost}")
    

# if __name__ == "__main__":
#     main()

# 测试示例
if __name__ == "__main__":
    # 示例距离矩阵（4个城市）
    distance_matrix = [
        [0, 10, 15, 20],
        [10, 0, 35, 25],
        [15, 35, 0, 30],
        [20, 25, 30, 0]
    ]
    
    tsp_solver = TSP_BFS(distance_matrix)
    path, cost = tsp_solver.solve()
    print(f"最优路径: {' -> '.join(map(str, path))}")
# 1. path 是一个列表，例如 [0, 1, 2, 3, 0]
# 2. map(str, path) 将path中的每个元素转换为字符串，因为join要求元素都是字符串
# 3. ' -> '.join(...) 用字符串' -> '将转换后的字符串列表连接起来，例如 "0 -> 1 -> 2 -> 3 -> 0"
# 4. 最后，f-string将这部分嵌入到字符串中，形成最终输出：最优路径: 0 -> 1 -> 2 -> 3 -> 0
    print(f"最短距离: {cost}")
