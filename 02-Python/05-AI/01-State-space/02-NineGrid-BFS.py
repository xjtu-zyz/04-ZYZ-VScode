#Blind Search
from collections import deque
import copy

class NineGrid_BFS:
    def __init__(self, initial_state, goal_state):
        self.initial_state = initial_state
        self.goal_state = goal_state
        self.n = 3  # 3x3棋盘
        self.directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]  # 右,下,左,上
        
    def find_blank(self, state): #找到空白块(0)的位置
        for i in range(self.n):
            for j in range(self.n):
                if state[i][j] == 0:
                    return i, j
        return -1, -1
    
    def is_goal(self, state):
        return state == self.goal_state
    
    def state_to_tuple(self, state):
        #将状态转换为元组以便哈希
        return tuple(tuple(row) for row in state)
    
    def get_neighbors(self, state):
        #获取所有可能的下一步状态
        blank_i, blank_j = self.find_blank(state)
        neighbors = []
        
        for di, dj in self.directions:
            new_i, new_j = blank_i + di, blank_j + dj
            
            if 0 <= new_i < self.n and 0 <= new_j < self.n:
                # 创建新状态
                new_state = copy.deepcopy(state)
                # 交换空白块和目标位置
                new_state[blank_i][blank_j], new_state[new_i][new_j] = \
                    new_state[new_i][new_j], new_state[blank_i][blank_j]
                neighbors.append(new_state)
                
        return neighbors
    
    def solve(self):
        # 使用BFS算法解决重排九宫格问题
        
        # 使用队列存储状态和路径
        # 队列元素: (state, path)
        initial_path = [self.initial_state]
        queue = deque([(self.initial_state, initial_path)])
        visited = set()
        visited.add(self.state_to_tuple(self.initial_state))
        
        while queue:
            current_state, path = queue.popleft()
            
            if self.is_goal(current_state):
                return path
            
            for neighbor in self.get_neighbors(current_state):
                neighbor_tuple = self.state_to_tuple(neighbor)
                if neighbor_tuple not in visited:
                    visited.add(neighbor_tuple)
                    new_path = path + [neighbor]
                    queue.append((neighbor, new_path))
        
        return None  # 无解

# 测试示例
if __name__ == "__main__":
    # 初始状态和目标状态
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
    
    puzzle_solver = NineGrid_BFS(initial, goal)
    solution = puzzle_solver.solve()
    
    if solution:
        print("BFS找到解决方案:")
        for i, state in enumerate(solution):
            print(f"步骤 {i}:")
            for row in state:
                print(row)
            print()
    else:
        print("BFS未找到解决方案")