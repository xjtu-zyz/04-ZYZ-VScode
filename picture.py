# from pylab import mpl
# mpl.rcParams['font.sans-serif'] = ['STZhongsong']    # 指定默认字体：解决plot不能显示中文问题
# mpl.rcParams['axes.unicode_minus'] = False           # 解决保存图像是负号'-'显示为方块的问题

import matplotlib.pyplot as plt
import numpy as np

# 创建数据点
x = np.linspace(-2*np.pi, 2*np.pi, 1000)  # 从-2π到2π生成1000个点
y_sin = np.sin(x)
y_cos = np.cos(x)

# 创建画布和坐标系
plt.figure(figsize=(10, 6))
ax = plt.subplot(1, 1, 1)

# 绘制正弦和余弦曲线
plt.plot(x, y_sin, color='blue', linewidth=2, label='sin(x)')
plt.plot(x, y_cos, color='red', linewidth=2, linestyle='--', label='cos(x)')

# 设置标题和标签
plt.title('正弦函数与余弦函数对比图', fontsize=14)
plt.xlabel('x (弧度)')
plt.ylabel('y 值')

# 设置坐标轴范围
plt.xlim(-2*np.pi, 2*np.pi)
plt.ylim(-1.5, 1.5)

# 设置x轴刻度为π的倍数
plt.xticks([-2*np.pi, -3*np.pi/2, -np.pi, -np.pi/2, 0, np.pi/2, np.pi, 3*np.pi/2, 2*np.pi],
           [r'-2π', r'-3π/2', r'-π', r'-π/2', '0', r'π/2', r'π', r'3π/2', r'2π'])

# 添加网格和图例
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(loc='upper right')

# 添加脊柱（坐标轴）
ax.spines['left'].set_position('zero')
ax.spines['bottom'].set_position('zero')
ax.spines['right'].set_color('none')
ax.spines['top'].set_color('none')

# 显示图形
plt.tight_layout()
plt.show()