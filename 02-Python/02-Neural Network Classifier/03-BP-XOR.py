import numpy as np
import time

class SimpleBPNeuralNetwork:
    """
    纯文本实现的BP神经网络 - 解决XOR问题
    不使用任何图像库，只输出文本结果
    """
    
    def __init__(self, input_size=2, hidden_size=4, output_size=1):
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.output_size = output_size
        
        # 初始化权重 - 使用更好的初始化方法
        np.random.seed(42)  # 固定随机种子，确保可重复性
        
        # Xavier初始化
        limit1 = np.sqrt(6.0 / (input_size + hidden_size))
        self.W1 = np.random.uniform(-limit1, limit1, (input_size, hidden_size))
        self.b1 = np.zeros((1, hidden_size))
        
        limit2 = np.sqrt(6.0 / (hidden_size + output_size))
        self.W2 = np.random.uniform(-limit2, limit2, (hidden_size, output_size))
        self.b2 = np.zeros((1, output_size))
        
        # 训练历史记录
        self.loss_history = []
        self.accuracy_history = []
        
        print(f"网络结构: {input_size} -> {hidden_size} -> {output_size}")
        print(f"权重初始化范围: W1=[{self.W1.min():.4f}, {self.W1.max():.4f}], "
              f"W2=[{self.W2.min():.4f}, {self.W2.max():.4f}]")
    
    def sigmoid(self, x):
        """Sigmoid激活函数"""
        x = np.clip(x, -500, 500)  # 防止数值溢出
        return 1.0 / (1.0 + np.exp(-x))
    
    def sigmoid_derivative(self, x):
        """Sigmoid导数 - 输入是sigmoid的输出"""
        return x * (1.0 - x)
    
    def tanh(self, x):
        """Tanh激活函数"""
        return np.tanh(x)
    
    def tanh_derivative(self, x):
        """Tanh导数 - 输入是tanh的输出"""
        return 1.0 - x**2
    
    def forward(self, X):
        """前向传播"""
        # 输入层到隐藏层 (使用tanh)
        self.z1 = np.dot(X, self.W1) + self.b1
        self.a1 = self.tanh(self.z1)
        
        # 隐藏层到输出层 (使用sigmoid)
        self.z2 = np.dot(self.a1, self.W2) + self.b2
        self.a2 = self.sigmoid(self.z2)
        
        return self.a2
    
    def compute_loss(self, y_true, y_pred):
        """计算交叉熵损失"""
        epsilon = 1e-15
        y_pred = np.clip(y_pred, epsilon, 1 - epsilon)
        loss = -np.mean(y_true * np.log(y_pred) + (1 - y_true) * np.log(1 - y_pred))
        return loss
    
    def backward(self, X, y_true, y_pred, learning_rate):
        """反向传播"""
        m = X.shape[0]  # 样本数量
        
        # 输出层梯度 (交叉熵损失 + sigmoid的简化形式)
        d_output = (y_pred - y_true) / m
        
        # 隐藏层到输出层梯度
        dW2 = np.dot(self.a1.T, d_output)
        db2 = np.sum(d_output, axis=0, keepdims=True)
        
        # 隐藏层梯度
        d_hidden = np.dot(d_output, self.W2.T) * self.tanh_derivative(self.a1)
        
        # 输入层到隐藏层梯度
        dW1 = np.dot(X.T, d_hidden)
        db1 = np.sum(d_hidden, axis=0, keepdims=True)
        
        # 梯度裁剪 (防止梯度爆炸)
        clip_value = 1.0
        dW2 = np.clip(dW2, -clip_value, clip_value)
        db2 = np.clip(db2, -clip_value, clip_value)
        dW1 = np.clip(dW1, -clip_value, clip_value)
        db1 = np.clip(db1, -clip_value, clip_value)
        
        # 更新权重
        self.W2 -= learning_rate * dW2
        self.b2 -= learning_rate * db2
        self.W1 -= learning_rate * dW1
        self.b1 -= learning_rate * db1
        
        # 返回梯度信息用于监控
        return {
            'dW2_max': np.abs(dW2).max(),
            'dW1_max': np.abs(dW1).max(),
            'output_grad_max': np.abs(d_output).max()
        }
    
    def predict(self, X, threshold=0.5):
        """预测"""
        output = self.forward(X)
        return (output > threshold).astype(int)
    
    def train(self, X_train, y_train, epochs=5000, learning_rate=0.8, verbose=True):
        """训练神经网络"""
        print(f"\n开始训练: epochs={epochs}, learning_rate={learning_rate}")
        print("=" * 70)
        
        start_time = time.time()
        
        for epoch in range(epochs):
            # 前向传播
            y_pred = self.forward(X_train)
            
            # 计算损失和准确率
            loss = self.compute_loss(y_train, y_pred)
            predictions = self.predict(X_train)
            accuracy = np.mean(predictions == y_train)
            
            self.loss_history.append(loss)
            self.accuracy_history.append(accuracy)
            
            # 反向传播
            grad_info = self.backward(X_train, y_train, y_pred, learning_rate)
            
            # 学习率衰减
            if epoch > 0 and epoch % 1000 == 0:
                learning_rate *= 0.9
                if verbose:
                    print(f"  [epoch {epoch}] 学习率衰减到: {learning_rate:.4f}")
            
            # 每500轮打印一次训练信息
            if verbose and epoch % 500 == 0:
                print(f"Epoch {epoch:5d}: Loss = {loss:.6f}, "
                      f"Accuracy = {accuracy:.4f}, "
                      f"Grad = [W2:{grad_info['dW2_max']:.2e}, "
                      f"W1:{grad_info['dW1_max']:.2e}]")
            
            # 如果准确率达到100%，提前停止
            if accuracy >= 1.0:
                print(f"\n✓ 提前停止! 在第{epoch}轮达到100%准确率")
                break
        
        end_time = time.time()
        
        print("\n" + "=" * 70)
        print("训练完成!")
        print(f"总时间: {end_time - start_time:.2f}秒")
        print(f"训练轮数: {len(self.loss_history)}")
        print(f"最终损失: {self.loss_history[-1]:.6f}")
        print(f"最终准确率: {self.accuracy_history[-1]:.4f}")
        
        # 显示训练过程摘要
        print(f"\n训练过程摘要:")
        print(f"初始损失: {self.loss_history[0]:.6f}")
        print(f"最终损失: {self.loss_history[-1]:.6f}")
        print(f"损失减少: {self.loss_history[0] - self.loss_history[-1]:.6f}")
        print(f"准确率提升: {self.accuracy_history[-1] - self.accuracy_history[0]:.4f}")
    
    def print_weights(self):
        """打印权重信息"""
        print("\n" + "=" * 70)
        print("权重和偏置信息")
        print("=" * 70)
        
        print("W1 (输入层 → 隐藏层):")
        for i in range(self.W1.shape[0]):
            print(f"  输入神经元{i}: {self.W1[i]}")
        
        print(f"\nb1 (隐藏层偏置): {self.b1.flatten()}")
        
        print("\nW2 (隐藏层 → 输出层):")
        for i in range(self.W2.shape[0]):
            print(f"  隐藏神经元{i}: {self.W2[i]}")
        
        print(f"\nb2 (输出层偏置): {self.b2.flatten()}")
    
    def print_predictions(self, X, y):
        """打印预测结果"""
        print("\n" + "=" * 70)
        print("测试结果")
        print("=" * 70)
        
        correct_count = 0
        total_count = len(X)
        
        for i in range(total_count):
            input_data = X[i:i+1, :]
            prediction = self.predict(input_data)
            output_prob = self.forward(input_data)
            target = y[i, 0]
            
            is_correct = prediction[0, 0] == target
            status = "✓" if is_correct else "✗"
            
            print(f"{status} 输入: {X[i]}, 目标: {target}, "
                  f"预测: {prediction[0,0]}, 概率: {output_prob[0,0]:.4f}")
            
            if is_correct:
                correct_count += 1
        
        accuracy = correct_count / total_count
        print(f"\n整体准确率: {accuracy:.4f} ({correct_count}/{total_count})")
        
        return accuracy

def create_xor_dataset():
    """创建XOR数据集"""
    X = np.array([[0, 0],
                  [0, 1],
                  [1, 0],
                  [1, 1]])
    
    y = np.array([[0],
                  [1],
                  [1],
                  [0]])
    
    return X, y

def demonstrate_backpropagation_step():
    """演示反向传播的单步计算"""
    print("\n" + "=" * 70)
    print("反向传播单步计算演示")
    print("=" * 70)
    
    # 创建简单的演示网络
    demo_nn = SimpleBPNeuralNetwork(input_size=2, hidden_size=2, output_size=1)
    
    # 设置演示用权重
    demo_nn.W1 = np.array([[0.5, -0.3],
                           [0.2, 0.8]])
    demo_nn.b1 = np.array([[0.1, -0.1]])
    demo_nn.W2 = np.array([[0.7],
                           [-0.5]])
    demo_nn.b2 = np.array([[0.2]])
    
    # 单个训练样本
    X_sample = np.array([[1, 0]])
    y_sample = np.array([[1]])
    
    print(f"输入: {X_sample[0]}")
    print(f"目标: {y_sample[0, 0]}")
    
    # 前向传播
    print("\n1. 前向传播:")
    output = demo_nn.forward(X_sample)
    print(f"   隐藏层输入 z1: {demo_nn.z1[0]}")
    print(f"   隐藏层输出 a1: {demo_nn.a1[0]}")
    print(f"   输出层输入 z2: {demo_nn.z2[0, 0]:.4f}")
    print(f"   输出层输出 a2: {output[0, 0]:.4f}")
    
    # 计算损失
    loss = demo_nn.compute_loss(y_sample, output)
    print(f"\n2. 计算损失:")
    print(f"   交叉熵损失: {loss:.6f}")
    
    # 反向传播
    print(f"\n3. 反向传播 (学习率=0.5):")
    grad_info = demo_nn.backward(X_sample, y_sample, output, 0.5)
    
    print(f"   输出层梯度 d_output: {output[0, 0] - y_sample[0, 0]:.6f}")
    print(f"   W2梯度范围: {grad_info['dW2_max']:.6f}")
    print(f"   W1梯度范围: {grad_info['dW1_max']:.6f}")
    
    print(f"\n4. 更新后的权重:")
    print(f"   W1: {demo_nn.W1.flatten()}")
    print(f"   W2: {demo_nn.W2.flatten()}")
    
    print(f"\n5. 预测结果:")
    prediction = demo_nn.predict(X_sample)
    print(f"   预测类别: {prediction[0, 0]}")

def analyze_training_history(nn):
    """分析训练历史"""
    print("\n" + "=" * 70)
    print("训练历史分析")
    print("=" * 70)
    
    if len(nn.loss_history) == 0:
        print("无训练历史数据")
        return
    
    # 计算关键统计信息
    epochs = len(nn.loss_history)
    loss_change = nn.loss_history[0] - nn.loss_history[-1]
    accuracy_change = nn.accuracy_history[-1] - nn.accuracy_history[0]
    
    print(f"总训练轮数: {epochs}")
    print(f"初始损失: {nn.loss_history[0]:.6f}")
    print(f"最终损失: {nn.loss_history[-1]:.6f}")
    print(f"损失减少量: {loss_change:.6f}")
    print(f"损失减少百分比: {(loss_change / nn.loss_history[0]) * 100:.1f}%")
    
    print(f"\n初始准确率: {nn.accuracy_history[0]:.4f}")
    print(f"最终准确率: {nn.accuracy_history[-1]:.4f}")
    print(f"准确率提升: {accuracy_change:.4f}")
    
    # 分析收敛情况
    if epochs >= 10:
        last_10_losses = nn.loss_history[-10:]
        loss_std = np.std(last_10_losses)
        print(f"\n最后10轮损失标准差: {loss_std:.6f}")
        
        if loss_std < 0.0001:
            print("状态: 已收敛 ✓")
        elif loss_std < 0.001:
            print("状态: 基本收敛")
        else:
            print("状态: 仍在波动")
    
    # 找到准确率达到1.0的轮次
    for i, acc in enumerate(nn.accuracy_history):
        if acc >= 1.0:
            print(f"首次达到100%准确率的轮次: {i}")
            break
    
    # 打印每1000轮的损失和准确率
    print(f"\n训练过程关键点:")
    for epoch in range(0, min(epochs, 10001), 1000):
        if epoch < epochs:
            print(f"  Epoch {epoch}: Loss={nn.loss_history[epoch]:.6f}, "
                  f"Acc={nn.accuracy_history[epoch]:.4f}")

def main():
    """
    主函数：使用BP神经网络解决XOR问题
    """
    print("=" * 80)
    print("BP神经网络实现 - XOR问题解决方案")
    print("=" * 80)
    
    # 1. 创建数据集
    print("\n[1/5] 创建数据集")
    X, y = create_xor_dataset()
    print(f"XOR数据集:")
    for i in range(len(X)):
        print(f"  {X[i]} -> {y[i, 0]}")
    
    # 2. 创建神经网络
    print("\n[2/5] 初始化神经网络")
    nn = SimpleBPNeuralNetwork(input_size=2, hidden_size=4, output_size=1)
    
    # 3. 演示单步反向传播
    print("\n[3/5] 演示单步反向传播")
    demonstrate_backpropagation_step()
    
    # 4. 训练神经网络
    print("\n[4/5] 训练神经网络")
    nn.train(X, y, epochs=5000, learning_rate=0.8, verbose=True)
    
    # 5. 测试神经网络
    print("\n[5/5] 测试神经网络")
    final_accuracy = nn.print_predictions(X, y)
    
    # 6. 分析训练历史
    analyze_training_history(nn)
    
    # 7. 显示权重信息
    nn.print_weights()
    
    # 8. 最终总结
    print("\n" + "=" * 80)
    print("实验总结")
    print("=" * 80)
    
    if final_accuracy >= 1.0:
        print("✓ 成功: BP神经网络完美解决了XOR问题!")
    else:
        print("✗ 失败: 未能完全解决XOR问题")
        print("可能的原因:")
        print("  1. 学习率不合适")
        print("  2. 网络结构过小")
        print("  3. 训练轮数不足")
        print("  4. 权重初始化不佳")
    
    print(f"\n最终配置:")
    print(f"  网络结构: 2 -> 4 -> 1")
    print(f"  激活函数: 隐藏层(tanh), 输出层(sigmoid)")
    print(f"  损失函数: 交叉熵")
    print(f"  优化算法: 梯度下降")
    
    return nn

def run_multiple_experiments():
    """运行多次实验，比较不同参数的效果"""
    print("\n" + "=" * 80)
    print("多组参数实验对比")
    print("=" * 80)
    
    X, y = create_xor_dataset()
    
    experiments = [
        {"hidden_size": 2, "learning_rate": 0.5, "epochs": 3000},
        {"hidden_size": 4, "learning_rate": 0.8, "epochs": 5000},
        {"hidden_size": 8, "learning_rate": 0.3, "epochs": 7000},
        {"hidden_size": 2, "learning_rate": 1.0, "epochs": 4000},
    ]
    
    results = []
    
    for i, exp in enumerate(experiments):
        print(f"\n实验 {i+1}: hidden_size={exp['hidden_size']}, "
              f"lr={exp['learning_rate']}, epochs={exp['epochs']}")
        
        nn = SimpleBPNeuralNetwork(
            input_size=2, 
            hidden_size=exp['hidden_size'], 
            output_size=1
        )
        
        # 训练
        nn.train(X, y, epochs=exp['epochs'], 
                learning_rate=exp['learning_rate'], verbose=False)
        
        # 测试
        predictions = nn.predict(X)
        accuracy = np.mean(predictions == y)
        
        results.append({
            "exp": exp,
            "accuracy": accuracy,
            "final_loss": nn.loss_history[-1],
            "epochs_needed": len(nn.loss_history)
        })
        
        print(f"  结果: 准确率={accuracy:.4f}, 最终损失={nn.loss_history[-1]:.6f}")
    
    # 显示比较结果
    print("\n" + "=" * 80)
    print("实验结果比较")
    print("=" * 80)
    
    print("排名 | 隐藏层大小 | 学习率 | 训练轮数 | 准确率 | 最终损失")
    print("-" * 70)
    
    sorted_results = sorted(results, key=lambda x: x['accuracy'], reverse=True)
    
    for i, res in enumerate(sorted_results):
        exp = res['exp']
        print(f"{i+1:2d}   | {exp['hidden_size']:10d} | {exp['learning_rate']:7.2f} | "
              f"{res['epochs_needed']:8d} | {res['accuracy']:7.4f} | {res['final_loss']:.6f}")
    
    best_result = sorted_results[0]
    print(f"\n最佳配置: hidden_size={best_result['exp']['hidden_size']}, "
          f"learning_rate={best_result['exp']['learning_rate']}")
    print(f"最佳准确率: {best_result['accuracy']:.4f}")

if __name__ == "__main__":
    print("程序开始时间:", time.strftime("%Y-%m-%d %H:%M:%S"))
    print()
    
    # 运行主实验
    trained_model = main()
    
    # 询问是否运行多组实验
    print("\n是否运行多组参数对比实验? (y/n)")
    user_input = input().strip().lower()
    
    if user_input == 'y':
        run_multiple_experiments()
    
    print("\n程序结束时间:", time.strftime("%Y-%m-%d %H:%M:%S"))
    print("实验完成!")