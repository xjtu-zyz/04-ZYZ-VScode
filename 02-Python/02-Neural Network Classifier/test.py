import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import make_classification
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelBinarizer

class SimpleNeuralNetwork:
    def __init__(self, input_size, hidden_size, output_size, learning_rate=0.1):
        self.weights1 = np.random.randn(input_size, hidden_size) * 0.1
        self.weights2 = np.random.randn(hidden_size, output_size) * 0.1
        self.bias1 = np.zeros((1, hidden_size))
        self.bias2 = np.zeros((1, output_size))
        self.lr = learning_rate
    
    def sigmoid(self, x):
        return 1 / (1 + np.exp(-np.clip(x, -250, 250)))
    
    def sigmoid_derivative(self, x):
        return x * (1 - x)
    
    def forward(self, X):
        self.hidden = self.sigmoid(np.dot(X, self.weights1) + self.bias1)
        self.output = self.sigmoid(np.dot(self.hidden, self.weights2) + self.bias2)
        return self.output
    
    def backward(self, X, y, output):
        m = X.shape[0]
        
        # 输出层误差
        d_output = (output - y) * self.sigmoid_derivative(output)
        
        # 隐藏层误差
        d_hidden = np.dot(d_output, self.weights2.T) * self.sigmoid_derivative(self.hidden)
        
        # 更新权重和偏置
        self.weights2 -= self.lr * np.dot(self.hidden.T, d_output) / m
        self.bias2 -= self.lr * np.sum(d_output, axis=0, keepdims=True) / m
        self.weights1 -= self.lr * np.dot(X.T, d_hidden) / m
        self.bias1 -= self.lr * np.sum(d_hidden, axis=0, keepdims=True) / m
    
    def train(self, X, y, epochs=1000):
        losses = []
        for i in range(epochs):
            output = self.forward(X)
            loss = np.mean((output - y) ** 2)
            losses.append(loss)
            self.backward(X, y, output)
            
            if i % 100 == 0:
                print(f"Epoch {i}, Loss: {loss:.4f}")
        
        return losses

# 创建简单的二分类数据集
X, y = make_classification(n_samples=1000, n_features=20, n_classes=2, 
                          n_informative=15, random_state=42)
y = y.reshape(-1, 1)

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 创建并训练网络
nn = SimpleNeuralNetwork(input_size=20, hidden_size=10, output_size=1, learning_rate=0.1)
losses = nn.train(X_train, y_train, epochs=500)

# 测试
predictions = nn.forward(X_test) > 0.5
accuracy = np.mean(predictions == y_test)
print(f"测试准确率: {accuracy:.4f}")

# 绘制损失曲线
plt.plot(losses)
plt.title('Training Loss')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.show()