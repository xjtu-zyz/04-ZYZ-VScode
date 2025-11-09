# Heuristic Search
import heapq
import numpy as np
from collections import deque
from typing import List, Tuple

class TSP_AStar:
    def __init__(self, distance_matrix):
        self.distance_matrix = distance_matrix
        self.n = len(distance_matrix)
        self.best_path = None
        self.best_cost = float('inf')
        
    def heuristic(self, current_city: int, visited: List[bool]) -> float:
        """
        启发式函数：使用最小生成树(MST)启发式
        这里简化使用最近邻启发式
        """
        unvisited = [i for i in range(self.n) if not visited[i]]
        if not unvisited:
            return 0
            
        # 计算当前城市到未访问城市的最小距离
        min_distances = []
        for city in unvisited:
            min_dist = min([self.distance_matrix[city][other] 
                          for other in unvisited if other != city], default=0)
            min_distances.append(min_dist)
            
        return sum(min_distances)
    
    def solve(self, start_city=0):
        """使用A*算法解决TSP问题"""
        # 优先级队列: (f_score, g_score, current_city, visited, path)
        # f_score = g_score + h_score
        visited = [False] * self.n
        visited[start_city] = True
        
        initial_state = (0, 0, start_city, visited[:], [start_city])
        open_set = [initial_state]
        heapq.heapify(open_set)
        
        best_cost = float('inf')
        best_path = None
        
        while open_set:
            f_score, g_score, current_city, visited, path = heapq.heappop(open_set)
            
            # 如果已经找到完整路径
            if len(path) == self.n:
                total_cost = g_score + self.distance_matrix[path[-1]][start_city]
                if total_cost < best_cost:
                    best_cost = total_cost
                    best_path = path + [start_city]
                continue
            
            # 如果当前代价已经超过已知最优解，剪枝
            if g_score >= best_cost:
                continue
                
            # 扩展当前节点的邻居
            for next_city in range(self.n):
                if not visited[next_city]:
                    new_g_score = g_score + self.distance_matrix[current_city][next_city]
                    new_visited = visited[:]
                    new_visited[next_city] = True
                    new_path = path + [next_city]
                    
                    # 计算启发式估值
                    h_score = self.heuristic(next_city, new_visited)
                    f_score = new_g_score + h_score
                    
                    if f_score < best_cost:
                        heapq.heappush(open_set, 
                                     (f_score, new_g_score, next_city, new_visited, new_path))
        
        return best_path, best_cost

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


# # 测试示例
# if __name__ == "__main__":

#     dist_matrix = [
#         [0, 10, 15, 20],
#         [10, 0, 35, 25],
#         [15, 35, 0, 30],
#         [20, 25, 30, 0]
#     ]
    
#     tsp_astar = TSP_AStar(dist_matrix)
#     path, cost = tsp_astar.solve()
#     print(f"最优路径: {' -> '.join(map(str, path))}")
#     print(f"最短距离: {cost}")



def main():
    """主函数"""
    print("TSP问题求解器 (使用A*算法)")
    
    distance_matrix = input_distance_matrix()

    # 打印距离矩阵
    print("\n距离矩阵:")
    for i, row in enumerate(distance_matrix):
        print(f"城市{i}: {row}")
    
    # 选择起始城市
    while True:
        try:
            start_city = int(input(f"\n请输入起始城市 (0-{len(distance_matrix)-1}): "))
            if 0 <= start_city < len(distance_matrix):
                break
            else:
                print(f"起始城市必须在 0-{len(distance_matrix)-1} 范围内")
        except ValueError:
            print("请输入有效的整数")
    
    # 求解TSP问题
    tsp_solver = TSP_AStar(distance_matrix)
    path, cost = tsp_solver.solve(start_city)
    
    # 输出结果
    print("\n" )
    print("计算结果:")
    print(f"最优路径: {' -> '.join(map(str, path))}")
    print(f"最短距离: {cost}")
    

if __name__ == "__main__":
    main()
