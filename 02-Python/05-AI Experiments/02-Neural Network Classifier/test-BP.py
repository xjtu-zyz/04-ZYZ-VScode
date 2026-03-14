import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import fetch_openml
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelBinarizer
import time

class NeuralNetwork:
    def __init__(self, layers, learning_rate=0.1):
        """
        初始化神经网络
        
        参数:
        layers: 列表，包含每层的神经元数量，例如[784, 128, 64, 10]
        learning_rate: 学习率
        """
        self.layers = layers
        self.learning_rate = learning_rate
        self.weights = []
        self.biases = []
        
        # 初始化权重和偏置
        for i in range(len(layers) - 1):
            # 使用Xavier初始化权重
            w = np.random.randn(layers[i], layers[i+1]) * np.sqrt(2.0 / layers[i])
            b = np.zeros((1, layers[i+1]))
            self.weights.append(w)
            self.biases.append(b)
    
    def sigmoid(self, x):
        """Sigmoid激活函数"""
        return 1 / (1 + np.exp(-np.clip(x, -250, 250)))  # 防止溢出
    
    def sigmoid_derivative(self, x):
        """Sigmoid函数的导数"""
        return x * (1 - x)
    
    def softmax(self, x):
        """Softmax函数，用于多分类输出层"""
        exp_x = np.exp(x - np.max(x, axis=1, keepdims=True))  # 防止溢出
        return exp_x / np.sum(exp_x, axis=1, keepdims=True)
    
    def forward(self, X):
        """前向传播"""
        self.activations = [X]  # 存储每层的激活值
        self.z_values = []      # 存储每层的加权输入
        
        # 隐藏层的前向传播
        for i in range(len(self.weights) - 1):
            z = np.dot(self.activations[-1], self.weights[i]) + self.biases[i]
            self.z_values.append(z)
            a = self.sigmoid(z)
            self.activations.append(a)
        
        # 输出层的前向传播（使用softmax）
        z = np.dot(self.activations[-1], self.weights[-1]) + self.biases[-1]
        self.z_values.append(z)
        a = self.softmax(z)
        self.activations.append(a)
        
        return self.activations[-1]
    
    def backward(self, X, y, output):
        """反向传播"""
        m = X.shape[0]  # 样本数量
        
        # 计算输出层的误差
        delta = output - y
        
        # 反向传播误差
        for i in range(len(self.weights) - 1, 0, -1):
            # 计算当前层的误差
            dW = np.dot(self.activations[i].T, delta) / m
            db = np.sum(delta, axis=0, keepdims=True) / m
            
            # 更新权重和偏置
            self.weights[i] -= self.learning_rate * dW
            self.biases[i] -= self.learning_rate * db
            
            # 计算前一层的误差
            delta = np.dot(delta, self.weights[i].T) * self.sigmoid_derivative(self.activations[i])
        
        # 更新第一层的权重和偏置
        dW = np.dot(self.activations[0].T, delta) / m
        db = np.sum(delta, axis=0, keepdims=True) / m
        self.weights[0] -= self.learning_rate * dW
        self.biases[0] -= self.learning_rate * db
    
    def compute_loss(self, y_true, y_pred):
        """计算交叉熵损失"""
        # 添加一个小值防止log(0)
        epsilon = 1e-12
        y_pred = np.clip(y_pred, epsilon, 1. - epsilon)
        
        # 计算交叉熵损失
        loss = -np.sum(y_true * np.log(y_pred)) / y_true.shape[0]
        return loss
    
    def predict(self, X):
        """预测"""
        output = self.forward(X)
        return np.argmax(output, axis=1)
    
    def accuracy(self, X, y):
        """计算准确率"""
        predictions = self.predict(X)
        true_labels = np.argmax(y, axis=1)
        return np.mean(predictions == true_labels)
    
    def train(self, X_train, y_train, X_val, y_val, epochs=100, batch_size=32, verbose=True):
        """训练神经网络"""
        train_losses = []
        val_losses = []
        train_accuracies = []
        val_accuracies = []
        
        n_batches = int(np.ceil(X_train.shape[0] / batch_size))
        
        for epoch in range(epochs):
            # 随机打乱数据
            indices = np.random.permutation(X_train.shape[0])
            X_shuffled = X_train[indices]
            y_shuffled = y_train[indices]
            
            epoch_loss = 0
            
            # 小批量梯度下降
            for i in range(n_batches):
                start_idx = i * batch_size
                end_idx = min((i + 1) * batch_size, X_train.shape[0])
                
                X_batch = X_shuffled[start_idx:end_idx]
                y_batch = y_shuffled[start_idx:end_idx]
                
                # 前向传播
                output = self.forward(X_batch)
                
                # 计算损失
                batch_loss = self.compute_loss(y_batch, output)
                epoch_loss += batch_loss
                
                # 反向传播
                self.backward(X_batch, y_batch, output)
            
            # 计算验证集上的损失和准确率
            val_output = self.forward(X_val)
            val_loss = self.compute_loss(y_val, val_output)
            train_acc = self.accuracy(X_train, y_train)
            val_acc = self.accuracy(X_val, y_val)
            
            train_losses.append(epoch_loss / n_batches)
            val_losses.append(val_loss)
            train_accuracies.append(train_acc)
            val_accuracies.append(val_acc)
            
            if verbose and epoch % 10 == 0:
                print(f"Epoch {epoch}: Train Loss = {train_losses[-1]:.4f}, "
                      f"Val Loss = {val_losses[-1]:.4f}, "
                      f"Train Acc = {train_acc:.4f}, Val Acc = {val_acc:.4f}")
        
        return {
            'train_losses': train_losses,
            'val_losses': val_losses,
            'train_accuracies': train_accuracies,
            'val_accuracies': val_accuracies
        }

def load_and_preprocess_data():
    """加载和预处理MNIST数据"""
    print("正在加载MNIST数据集...")
    
    # 加载MNIST数据集
    mnist = fetch_openml('mnist_784', version=1, as_frame=False, parser='liac-arff')
    X, y = mnist.data, mnist.target.astype(int)
    
    # 归一化像素值到[0,1]范围
    X = X / 255.0
    
    # 将标签转换为one-hot编码
    lb = LabelBinarizer()
    y_onehot = lb.fit_transform(y)
    
    # 分割数据集
    X_train, X_test, y_train, y_test = train_test_split(
        X, y_onehot, test_size=0.2, random_state=42
    )
    
    # 从训练集中分出验证集
    X_train, X_val, y_train, y_val = train_test_split(
        X_train, y_train, test_size=0.1, random_state=42
    )
    
    print(f"训练集大小: {X_train.shape}")
    print(f"验证集大小: {X_val.shape}")
    print(f"测试集大小: {X_test.shape}")
    
    return X_train, X_val, X_test, y_train, y_val, y_test, lb

def plot_results(history):
    """绘制训练过程中的损失和准确率曲线"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4))
    
    # 绘制损失曲线
    ax1.plot(history['train_losses'], label='训练损失')
    ax1.plot(history['val_losses'], label='验证损失')
    ax1.set_title('训练和验证损失')
    ax1.set_xlabel('Epoch')
    ax1.set_ylabel('Loss')
    ax1.legend()
    ax1.grid(True)
    
    # 绘制准确率曲线
    ax2.plot(history['train_accuracies'], label='训练准确率')
    ax2.plot(history['val_accuracies'], label='验证准确率')
    ax2.set_title('训练和验证准确率')
    ax2.set_xlabel('Epoch')
    ax2.set_ylabel('Accuracy')
    ax2.legend()
    ax2.grid(True)
    
    plt.tight_layout()
    plt.show()

def visualize_predictions(model, X_test, y_test, lb, num_samples=10):
    """可视化一些预测结果"""
    # 随机选择一些测试样本
    indices = np.random.choice(X_test.shape[0], num_samples, replace=False)
    samples = X_test[indices]
    true_labels = np.argmax(y_test[indices], axis=1)
    
    # 预测
    predictions = model.predict(samples)
    
    # 可视化
    fig, axes = plt.subplots(2, 5, figsize=(12, 6))
    axes = axes.ravel()
    
    for i in range(num_samples):
        # 显示图像
        axes[i].imshow(samples[i].reshape(28, 28), cmap='gray')
        axes[i].set_title(f'真实: {true_labels[i]}, 预测: {predictions[i]}')
        axes[i].axis('off')
    
    plt.tight_layout()
    plt.show()

def main():
    """主函数"""
    # 加载数据
    X_train, X_val, X_test, y_train, y_val, y_test, lb = load_and_preprocess_data()
    
    # 创建神经网络
    # 网络结构: 784(输入) -> 128(隐藏层1) -> 64(隐藏层2) -> 10(输出)
    layers = [784, 64,32,10]
    nn = NeuralNetwork(layers, learning_rate=0.1)
    
    print(f"神经网络结构: {layers}")
    print(f"学习率: {nn.learning_rate}")
    
    # 训练神经网络
    print("开始训练神经网络...")
    start_time = time.time()
    
    history = nn.train(
        X_train, y_train, 
        X_val, y_val, 
        epochs=10, 
        batch_size=128, 
        verbose=True
    )
    
    end_time = time.time()
    print(f"训练完成! 耗时: {end_time - start_time:.2f}秒")
    
    # 在测试集上评估模型
    test_accuracy = nn.accuracy(X_test, y_test)
    print(f"测试集准确率: {test_accuracy:.4f}")
    
    # 绘制训练过程
    plot_results(history)
    
    # 可视化一些预测结果
    visualize_predictions(nn, X_test, y_test, lb)
    
    return nn, history

if __name__ == "__main__":
    model, history = main()