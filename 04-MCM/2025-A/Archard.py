import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm, multivariate_normal
from scipy.optimize import minimize


class StairWearSimulator:
    """
    楼梯磨损正向模拟器
    基于Archard模型模拟不同使用模式下的磨损形态
    """
    
    def __init__(self, material_hardness=3.5, wear_coefficient=1e-6, 
                 step_width=1.2, step_depth=0.3, n_points=100):
        """
        参数：
        material_hardness: 莫氏硬度（无量纲）
        wear_coefficient: 磨损系数（需校准）
        step_width: 踏步宽度（米）
        step_depth: 踏步深度（米）
        n_points: 空间离散点数
        """
        self.material_hardness = material_hardness
        self.wear_coefficient = wear_coefficient
        self.step_width = step_width
        self.step_depth = step_depth
        self.n_points = n_points
        
        # 空间网格
        self.x_grid = np.linspace(0, step_width, n_points)  # 宽度方向
        self.y_grid = np.linspace(0, step_depth, n_points)  # 深度方向
        self.X, self.Y = np.meshgrid(self.x_grid, self.y_grid)
        
    def pressure_distribution(self, n_people=1, direction='up', 
                              foot_position='center', x_offset=0):
        """
        计算单步压力分布
        考虑人体重量和脚底接触面积
        """
        # 平均体重70kg，接触面积0.03m²
        force_per_person = 70 * 9.81  # N
        contact_area = 0.03  # m²
        pressure = force_per_person / contact_area  # Pa
        
        # 空间分布：使用高斯分布模拟脚底压力
        if foot_position == 'center':
            center_x = self.step_width / 2 + x_offset
        elif foot_position == 'left':
            center_x = self.step_width / 4 + x_offset
        else:  # right
            center_x = 3 * self.step_width / 4 + x_offset
        
        # 标准差：模拟脚底尺寸
        sigma_x = 0.08  # 宽度方向
        sigma_y = 0.15  # 深度方向
        
        # 二维高斯分布
        pos = np.empty(self.X.shape + (2,))
        pos[:, :, 0] = self.X
        pos[:, :, 1] = self.Y
        
        rv = multivariate_normal([center_x, self.step_depth/2], 
                                [[sigma_x**2, 0], [0, sigma_y**2]])
        
        pressure_dist = pressure * rv.pdf(pos) * contact_area
        
        return pressure_dist
    
    def simulate_usage(self, total_users=100000, direction_ratio=0.6, 
                       parallel_usage=0.3, foot_offset=0.1):
        """
        模拟长期使用造成的磨损
        direction_ratio: 上行比例（0.5表示上下行相等）
        parallel_usage: 并行使用概率（0表示单行，1表示总是并行）
        """
        wear_cumulative = np.zeros((self.n_points, self.n_points))
        
        # 每次使用参数
        steps_per_user = 20  # 假设平均每人走20级台阶
        
        for _ in range(total_users):
            # 随机方向
            direction = 'up' if np.random.random() < direction_ratio else 'down'
            
            # 是否并行使用
            if np.random.random() < parallel_usage:
                # 两人并行：左+右
                positions = ['left', 'right']
            else:
                # 单行：随机位置
                positions = [np.random.choice(['left', 'center', 'right'])]
            
            for pos in positions:
                # 随机脚步偏移（模拟不同身高人群）
                offset = np.random.normal(0, foot_offset)
                
                # 计算此步的压力分布
                pressure = self.pressure_distribution(
                    n_people=1, direction=direction, 
                    foot_position=pos, x_offset=offset
                )
                
                # Archard模型：dV = k * F * L / H
                # 转换为深度：dh = k * p * dl / H
                # 假设每步滑动距离dl ~ 0.3m
                sliding_distance = self.step_depth * 0.8  # 实际接触长度
                wear_increment = (self.wear_coefficient * pressure * sliding_distance / 
                                 self.material_hardness)
                
                wear_cumulative += wear_increment
        
        return wear_cumulative
    
    def generate_measurement_data(self, wear_cumulative, noise_level=0.1):
        """
        添加测量噪声，模拟实际扫描数据
        """
        # 添加高斯噪声
        noise = np.random.normal(0, noise_level, wear_cumulative.shape)
        measured_wear = wear_cumulative + noise
        
        # 模拟边缘保护效应（实际中边缘磨损较少）
        edge_mask = (self.X < 0.05) | (self.X > self.step_width - 0.05)
        measured_wear[edge_mask] *= 0.3
        
        return measured_wear

# 使用示例
if __name__ == "__main__":
    simulator = StairWearSimulator(material_hardness=3.5, 
                                   wear_coefficient=2e-6,
                                   step_width=1.2, step_depth=0.3)
    
    # 模拟不同使用模式
    wear1 = simulator.simulate_usage(total_users=50000, direction_ratio=0.6,
                                    parallel_usage=0.0)  # 单行上行主导
    wear2 = simulator.simulate_usage(total_users=50000, direction_ratio=0.5,
                                    parallel_usage=0.5)  # 并行双向
    
    # 可视化
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    
    im1 = axes[0].contourf(simulator.X, simulator.Y, wear1, levels=20, cmap='viridis')
    axes[0].set_title('单行上行主导模式 (50k用户)')
    axes[0].set_xlabel('踏步宽度 (m)')
    axes[0].set_ylabel('踏步深度 (m)')
    plt.colorbar(im1, ax=axes[0], label='磨损深度 (mm)')
    
    im2 = axes[1].contourf(simulator.X, simulator.Y, wear2, levels=20, cmap='viridis')
    axes[1].set_title('并行双向模式 (50k用户)')
    axes[1].set_xlabel('踏步宽度 (m)')
    axes[1].set_ylabel('踏步深度 (m)')
    plt.colorbar(im2, ax=axes[1], label='磨损深度 (mm)')
    
    plt.tight_layout()
    plt.savefig('wear_patterns.png', dpi=300)
    plt.show()