import heapq
import copy
from typing import List, Tuple

class EightPuzzle_AStar:
    def __init__(self, initial_state, goal_state):
        self.initial_state = initial_state
        self.goal_state = goal_state
        self.n = 3
        
    def manhattan_distance(self, state):
        """计算曼哈顿距离启发式"""
        distance = 0
        for i in range(self.n):
            for j in range(self.n):
                value = state[i][j]
                if value != 0:  # 忽略空白块
                    # 找到该数字在目标状态中的位置
                    goal_i, goal_j = self.find_position(value, self.goal_state)
                    distance += abs(i - goal_i) + abs(j - goal_j)
        return distance
    
    def misplaced_tiles(self, state):
        """计算错位棋子数启发式"""
        count = 0
        for i in range(self.n):
            for j in range(self.n):
                if state[i][j] != 0 and state[i][j] != self.goal_state[i][j]:
                    count += 1
        return count
    
    def find_position(self, value, state):
        """找到指定值在状态中的位置"""
        for i in range(self.n):
            for j in range(self.n):
                if state[i][j] == value:
                    return i, j
        return -1, -1
    
    def find_blank(self, state):
        """找到空白块位置"""
        return self.find_position(0, state)
    
    def get_neighbors(self, state):
        """获取邻居状态"""
        blank_i, blank_j = self.find_blank(state)
        neighbors = []
        directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]
        
        for di, dj in directions:
            new_i, new_j = blank_i + di, blank_j + dj
            if 0 <= new_i < self.n and 0 <= new_j < self.n:
                new_state = copy.deepcopy(state)
                new_state[blank_i][blank_j], new_state[new_i][new_j] = \
                    new_state[new_i][new_j], new_state[blank_i][blank_j]
                neighbors.append(new_state)
                
        return neighbors
    
    def state_to_tuple(self, state):
        """状态转换为可哈希的元组"""
        return tuple(tuple(row) for row in state)
    
    def solve(self, heuristic='manhattan'):
        """使用A*算法解决重排九宫格问题"""
        if heuristic == 'manhattan':
            h_func = self.manhattan_distance
        else:
            h_func = self.misplaced_tiles
            
        # 初始状态的g_score和h_score
        g_score = 0
        h_score = h_func(self.initial_state)
        f_score = g_score + h_score
        
        # 优先级队列: (f_score, g_score, state, path)
        open_set = [(f_score, g_score, self.initial_state, [self.initial_state])]
        heapq.heapify(open_set)
        
        closed_set = set()
        
        while open_set:
            f_score, g_score, current_state, path = heapq.heappop(open_set)
            current_tuple = self.state_to_tuple(current_state)
            
            if current_state == self.goal_state:
                return path
                
            if current_tuple in closed_set:
                continue
                
            closed_set.add(current_tuple)
            
            # 扩展邻居
            for neighbor in self.get_neighbors(current_state):
                neighbor_tuple = self.state_to_tuple(neighbor)
                if neighbor_tuple not in closed_set:
                    new_g_score = g_score + 1
                    new_h_score = h_func(neighbor)
                    new_f_score = new_g_score + new_h_score
                    new_path = path + [neighbor]
                    
                    heapq.heappush(open_set, 
                                 (new_f_score, new_g_score, neighbor, new_path))
        
        return None  # 无解

# 测试示例
if __name__ == "__main__":
    initial = [
        [2, 8, 3],
        [1, 0, 4],
        [7, 6, 5]
    ]
    
    goal = [
        [1, 2, 3],
        [8, 0, 4],
        [7, 6, 5]
    ]
    
    puzzle_astar = EightPuzzle_AStar(initial, goal)
    solution = puzzle_astar.solve(heuristic='manhattan')
    
    if solution:
        print("A*算法找到解决方案:")
        for i, state in enumerate(solution):
            # # 使用曼哈顿距离作为启发式信息输出
            # print(f"步骤 {i} (g={i}, h={puzzle_astar.manhattan_distance(state)}):")
            # 使用错位棋子数作为启发式信息输出
            print(f"步骤 {i} (g={i}, h={puzzle_astar.misplaced_tiles(state)}):")
            for row in state:
                print(row)
            print()
    else:
        print("A*算法未找到解决方案")