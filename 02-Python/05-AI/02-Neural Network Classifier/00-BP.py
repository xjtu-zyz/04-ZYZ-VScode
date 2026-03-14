import numpy as np

# 激活函数及其导数
def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def sigmoid_derivative(x):
    """计算sigmoid函数的导数"""
    return x * (1 - x)

class BPNN:
    def __init__(self, input_size, hidden_size, output_size):
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.output_size = output_size
        
        # 初始化权重矩阵
        # 输入层到隐藏层权重: (input_size, hidden_size)是权重矩阵维度
        # 隐藏层到输出层权重: (hidden_size, output_size)
        self.weights_input_hidden = np.random.uniform(-0.5, 0.5, (self.input_size, self.hidden_size))
        self.weights_hidden_output = np.random.uniform(-0.5, 0.5, (self.hidden_size, self.output_size))
        
        # 初始化偏置项
        self.bias_hidden = np.zeros((1, self.hidden_size))
        self.bias_output = np.zeros((1, self.output_size))
        
        # print(f"权重形状: input->hidden: {self.weights_input_hidden.shape}, hidden->output: {self.weights_hidden_output.shape}")
        # print(f"偏置形状: hidden: {self.bias_hidden.shape}, output: {self.bias_output.shape}")
    
    def forward(self, inputs):
        """前向传播：三层神经网络"""
        # 保存输入用于反向传播
        self.inputs = inputs
        # 隐藏层计算：加权求和+偏置+激活函数
        self.hidden_input = np.dot(inputs, self.weights_input_hidden) + self.bias_hidden  
        self.hidden_output = sigmoid(self.hidden_input)  
        # 输出层计算
        self.final_input = np.dot(self.hidden_output, self.weights_hidden_output) + self.bias_output  
        self.final_output = sigmoid(self.final_input) 
        
        return self.final_output
    
    def compute_loss(self, targets):
        """计算损失函数 - 使用均方误差"""
        loss = 0.5 * np.sum((targets - self.final_output) ** 2)
        return loss
    
    def backward(self, targets, learning_rate):
        """反向传播"""
        # 获取样本数量
        m = 1  # 当前为单个样本
        
        # 1. 计算输出层误差和delta
        output_errors = (self.final_output-targets)  # 误差函数关于y'的导数
        output_deltas = output_errors * sigmoid_derivative(self.final_output)  # delta = 误差 * 激活函数导数
        
        # 2. 计算隐藏层误差和delta
        # 隐藏层误差 = 输出层delta * 隐藏层到输出层的权重转置
        hidden_errors = np.dot(output_deltas, self.weights_hidden_output.T) # 误差函数关于z的导数
        hidden_deltas = hidden_errors * sigmoid_derivative(self.hidden_output)  # delta = 误差 * 激活函数导数
        
        # 3. 更新权重和偏置
        # 更新隐藏层到输出层权重
        self.weights_hidden_output -= learning_rate * np.dot(self.hidden_output.T, output_deltas)
        self.bias_output -= learning_rate * np.sum(output_deltas, axis=0, keepdims=True)
        # 更新输入层到隐藏层权重
        self.weights_input_hidden -= learning_rate * np.dot(self.inputs.T, hidden_deltas)
        self.bias_hidden -= learning_rate * np.sum(hidden_deltas, axis=0, keepdims=True)
        
        # 返回当前样本的损失
        return self.compute_loss(targets)
    
    def train(self, data, labels, epochs=10000, learning_rate=0.1, verbose=True):
        """训练神经网络"""
        print(f"\n开始训练: epochs={epochs}, learning_rate={learning_rate}")
    
        
        loss_history = []
        for epoch in range(epochs):
            epoch_loss = 0
            # 遍历所有训练样本
            for i in range(len(data)):
                inputs = data[i:i+1, :]  # 保持2D形状 (1, input_size)
                targets = labels[i:i+1, :]  # 保持2D形状 (1, output_size)
                
                # 前向传播
                self.forward(inputs)
                # 反向传播并累积损失
                sample_loss = self.backward(targets, learning_rate)
                epoch_loss += sample_loss
            
            # 计算平均损失
            avg_loss = epoch_loss / len(data)
            loss_history.append(avg_loss)
            
            # 每1000轮打印一次训练信息
            if verbose and (epoch % 1000 == 0 or epoch == epochs - 1):
                print(f"Epoch {epoch:5d}: Loss = {avg_loss:.6f}")
            
            # 如果损失足够小，提前停止
            if avg_loss < 0.001:
                print(f"提前停止! 在第{epoch}轮达到目标损失")
                break
        
        print("\n训练完成!")
        return loss_history
    
    def predict(self, inputs, threshold=0.5):
        """预测函数"""
        output = self.forward(inputs)
        prediction = (output > threshold).astype(int)
        return prediction, output
    #决策转换（Decision Making）
    # 阈值判断：(output > threshold).astype(int)
    # 分类决策：将连续概率值转换为二元类别标签
    # .astype(int) 输出转换：float → int（0或1）

# 示例数据训练与测试
if __name__ == "__main__":
    print("BP神经网络 - XOR问题演示")
    
    # 输入数据和目标输出
    data = np.array([[0, 0], [0, 1], [1, 0], [1, 1]])
    labels = np.array([[0], [1], [1], [0]])  # XOR问题
 
    # 创建BP神经网络
    print(f"\n创建神经网络:")
    bpnn = BPNN(input_size=2, hidden_size=3, output_size=1)
    
    # 训练网络
    loss_history = bpnn.train(data, labels, epochs=8000, learning_rate=0.3)
    
    # 测试网络
    print(f"\n测试结果:")
    correct_predictions = 0
    total_predictions = len(data)
    
    for i, sample in enumerate(data):
        inputs = sample.reshape(1, -1)  # 转换为2D数组 (1, 2)
        prediction, probability = bpnn.predict(inputs)
        target = labels[i][0]
        # 检查预测是否正确
        is_correct = prediction[0, 0] == target
        if is_correct:
            correct_predictions += 1 

        print(f" 输入: {sample}, 目标: {target}, "
              f"预测: {prediction[0, 0]}, 输出: {probability[0, 0]:.4f}")
        
    accuracy = correct_predictions / total_predictions * 100
    print(f"\n准确率: {accuracy:.1f}% ({correct_predictions}/{total_predictions})")
    
