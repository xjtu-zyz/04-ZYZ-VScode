import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_breast_cancer
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import time
from pylab import mpl
mpl.rcParams['font.sans-serif'] = ['STZhongsong']    # 指定默认字体：解决plot不能显示中文问题
mpl.rcParams['axes.unicode_minus'] = False           # 解决保存图像是负号'-'显示为方块的问题
class BPNeuralNetwork:
    def __init__(self, layers, learning_rate=0.001, reg_lambda=0.001):
        """
        初始化BP神经网络
        
        参数:
        layers: 网络结构，例如[30, 16, 8, 1]表示输入层30个神经元，两个隐藏层16和8个神经元，输出层1个神经元
        learning_rate: 学习率
        reg_lambda: L2正则化参数
        """
        self.layers = layers
        self.learning_rate = learning_rate
        self.reg_lambda = reg_lambda
        
        # 初始化权重和偏置
        self.weights = []
        self.biases = []
        
        for i in range(len(layers) - 1):
            # 使用He初始化，适合ReLU激活函数
            w = np.random.randn(layers[i], layers[i+1]) * np.sqrt(2.0 / layers[i])
            b = np.zeros((1, layers[i+1]))
            self.weights.append(w)
            self.biases.append(b)
    
    def relu(self, x):
        """ReLU激活函数"""
        return np.maximum(0, x)
    
    def relu_derivative(self, x):
        """ReLU函数的导数"""
        return (x > 0).astype(float)
    
    def sigmoid(self, x):
        """Sigmoid激活函数用于输出层"""
        # 防止数值溢出
        x = np.clip(x, -250, 250)
        return 1 / (1 + np.exp(-x))
    
    def forward(self, X):
        """前向传播"""
        self.activations = [X]  # 存储每层的激活值
        self.z_values = []      # 存储每层的加权输入
        
        # 隐藏层的前向传播（使用ReLU）
        for i in range(len(self.weights) - 1):
            z = np.dot(self.activations[-1], self.weights[i]) + self.biases[i]
            self.z_values.append(z)
            a = self.relu(z)
            self.activations.append(a)
        
        # 输出层的前向传播（使用Sigmoid）
        z = np.dot(self.activations[-1], self.weights[-1]) + self.biases[-1]
        self.z_values.append(z)
        a = self.sigmoid(z)
        self.activations.append(a)
        
        return self.activations[-1]
    
    def compute_loss(self, y_true, y_pred):
        """计算二元交叉熵损失"""
        m = y_true.shape[0]
        # 添加小值防止log(0)
        epsilon = 1e-12
        y_pred = np.clip(y_pred, epsilon, 1. - epsilon)
        
        # 交叉熵损失
        cross_entropy = -np.mean(y_true * np.log(y_pred) + (1 - y_true) * np.log(1 - y_pred))
        
        # L2正则化项
        reg_term = 0
        for w in self.weights:
            reg_term += np.sum(np.square(w))
        reg_term = (self.reg_lambda / (2 * m)) * reg_term
        
        return cross_entropy + reg_term
    
    def backward(self, X, y, output):
        """反向传播"""
        m = X.shape[0]
        
        # 计算输出层的误差
        delta = output - y
        
        # 反向传播误差
        for i in range(len(self.weights) - 1, 0, -1):
            # 计算当前层的梯度（包含正则化项）
            dW = np.dot(self.activations[i].T, delta) / m + (self.reg_lambda / m) * self.weights[i]
            db = np.sum(delta, axis=0, keepdims=True) / m
            
            # 更新权重和偏置
            self.weights[i] -= self.learning_rate * dW
            self.biases[i] -= self.learning_rate * db
            
            # 计算前一层的误差（使用ReLU导数）
            delta = np.dot(delta, self.weights[i].T) * self.relu_derivative(self.activations[i])
        
        # 更新第一层的权重和偏置
        dW = np.dot(self.activations[0].T, delta) / m + (self.reg_lambda / m) * self.weights[0]
        db = np.sum(delta, axis=0, keepdims=True) / m
        self.weights[0] -= self.learning_rate * dW
        self.biases[0] -= self.learning_rate * db
    
    def predict(self, X, threshold=0.5):
        """预测类别"""
        output = self.forward(X)
        return (output > threshold).astype(int)
    
    def predict_proba(self, X):
        """预测概率"""
        return self.forward(X)
    
    def accuracy(self, X, y):
        """计算准确率"""
        predictions = self.predict(X)
        return accuracy_score(y, predictions)
    
    def train(self, X_train, y_train, X_val, y_val, epochs=1000, batch_size=32, verbose=True):
        """训练神经网络"""
        train_losses = []
        val_losses = []
        train_accuracies = []
        val_accuracies = []
        
        n_batches = int(np.ceil(X_train.shape[0] / batch_size))
        
        print("开始训练神经网络...")
        start_time = time.time()
        
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
            
            if verbose and epoch % 100 == 0:
                print(f"Epoch {epoch}: Train Loss = {train_losses[-1]:.4f}, "
                      f"Val Loss = {val_losses[-1]:.4f}, "
                      f"Train Acc = {train_acc:.4f}, Val Acc = {val_acc:.4f}")
        
        end_time = time.time()
        print(f"训练完成! 耗时: {end_time - start_time:.2f}秒")
        
        return {
            'train_losses': train_losses,
            'val_losses': val_losses,
            'train_accuracies': train_accuracies,
            'val_accuracies': val_accuracies
        }

def load_and_preprocess_data():
    """加载和预处理乳腺癌数据集"""
    print("正在加载乳腺癌数据集...")
    
    # 加载数据集
    data = load_breast_cancer()
    X, y = data.data, data.target
    
    # 打印数据集信息
    print(f"数据集特征数: {X.shape[1]}")
    print(f"数据集样本数: {X.shape[0]}")
    print(f"类别分布: 良性 {np.sum(y==1)}, 恶性 {np.sum(y==0)}")
    
    # 标准化特征
    scaler = StandardScaler()
    X = scaler.fit_transform(X)
    
    # 分割数据集
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y
    )
    
    # 从训练集中分出验证集
    X_train, X_val, y_train, y_val = train_test_split(
        X_train, y_train, test_size=0.1, random_state=42, stratify=y_train
    )
    
    # 将标签转换为列向量
    y_train = y_train.reshape(-1, 1)
    y_val = y_val.reshape(-1, 1)
    y_test = y_test.reshape(-1, 1)
    
    print(f"训练集大小: {X_train.shape}")
    print(f"验证集大小: {X_val.shape}")
    print(f"测试集大小: {X_test.shape}")
    
    return X_train, X_val, X_test, y_train, y_val, y_test, scaler, data

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

def evaluate_model(model, X_test, y_test):
    """评估模型性能"""
    # 预测
    y_pred = model.predict(X_test)
    y_pred_proba = model.predict_proba(X_test)
    
    # 计算准确率
    accuracy = accuracy_score(y_test, y_pred)
    print(f"测试集准确率: {accuracy:.4f}")
    
    # 分类报告
    print("\n分类报告:")
    print(classification_report(y_test, y_pred, target_names=['良性', '恶性']))
    
    # 混淆矩阵
    cm = confusion_matrix(y_test, y_pred)
    print("混淆矩阵:")
    print(cm)
    
    return accuracy, y_pred, y_pred_proba

def plot_roc_curve(y_test, y_pred_proba):
    """绘制ROC曲线"""
    from sklearn.metrics import roc_curve, auc
    
    fpr, tpr, thresholds = roc_curve(y_test, y_pred_proba)
    roc_auc = auc(fpr, tpr)
    
    plt.figure(figsize=(8, 6))
    plt.plot(fpr, tpr, color='darkorange', lw=2, label=f'ROC曲线 (AUC = {roc_auc:.4f})')
    plt.plot([0, 1], [0, 1], color='navy', lw=2, linestyle='--', label='随机分类器')
    plt.xlim([0.0, 1.0])
    plt.ylim([0.0, 1.05])
    plt.xlabel('假正率')
    plt.ylabel('真正率')
    plt.title('ROC曲线')
    plt.legend(loc="lower right")
    plt.grid(True)
    plt.show()
    
    return roc_auc

def main():
    """主函数"""
    # 加载数据
    X_train, X_val, X_test, y_train, y_val, y_test, scaler, data = load_and_preprocess_data()
    
    # 创建神经网络
    # 网络结构: 30(输入) -> 16(隐藏层1) -> 8(隐藏层2) -> 1(输出)
    layers = [X_train.shape[1], 16, 8, 1]
    nn = BPNeuralNetwork(layers, learning_rate=0.01, reg_lambda=0.001)
    
    print(f"\n神经网络结构: {layers}")
    print(f"学习率: {nn.learning_rate}")
    print(f"正则化参数: {nn.reg_lambda}")
    
    # 训练神经网络
    history = nn.train(
        X_train, y_train, 
        X_val, y_val, 
        epochs=1000, 
        batch_size=32, 
        verbose=True
    )
    
    # 评估模型
    print("\n模型评估:")
    accuracy, y_pred, y_pred_proba = evaluate_model(nn, X_test, y_test)
    
    # 绘制训练过程
    plot_results(history)
    
    # 绘制ROC曲线
    roc_auc = plot_roc_curve(y_test, y_pred_proba)
    print(f"\nAUC: {roc_auc:.4f}")
    
    return nn, history, accuracy, roc_auc

if __name__ == "__main__":
    model, history, accuracy, auc = main()