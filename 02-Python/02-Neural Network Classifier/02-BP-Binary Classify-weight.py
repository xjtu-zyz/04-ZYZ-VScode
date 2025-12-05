import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_breast_cancer
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix, roc_curve, auc
import time
import warnings
from pylab import mpl
mpl.rcParams['font.sans-serif'] = ['STZhongsong']    # 指定默认字体：解决plot不能显示中文问题
mpl.rcParams['axes.unicode_minus'] = False           # 解决保存图像是负号'-'显示为方块的问题
warnings.filterwarnings('ignore')

class CorrectedBPNeuralNetwork:
    def __init__(self, layers, learning_rate=0.01, reg_lambda=0.001, cost_matrix=None):
        """
        修正的BP神经网络
        
        参数:
        layers: 网络结构
        learning_rate: 学习率
        reg_lambda: 正则化参数
        cost_matrix: 代价矩阵 [TN_cost, FP_cost, FN_cost, TP_cost]
        """
        self.layers = layers
        self.learning_rate = learning_rate
        self.reg_lambda = reg_lambda
        self.cost_matrix = cost_matrix  # 代价矩阵
        
        # 初始化权重和偏置
        self.weights = []
        self.biases = []
        
        for i in range(len(layers) - 1):
            # 使用更稳定的初始化
            w = np.random.randn(layers[i], layers[i+1]) * np.sqrt(1.0 / layers[i])
            b = np.zeros((1, layers[i+1]))
            self.weights.append(w)
            self.biases.append(b)
    
    def relu(self, x):
        """ReLU激活函数"""
        return np.maximum(0, x)
    
    def relu_derivative(self, x):
        """ReLU导数"""
        return (x > 0).astype(float)
    
    def sigmoid(self, x):
        """Sigmoid函数"""
        x = np.clip(x, -250, 250)
        return 1 / (1 + np.exp(-x))
    
    def forward(self, X):
        """前向传播"""
        self.activations = [X]
        self.z_values = []
        
        # 隐藏层
        for i in range(len(self.weights) - 1):
            z = np.dot(self.activations[-1], self.weights[i]) + self.biases[i]
            self.z_values.append(z)
            a = self.relu(z)
            self.activations.append(a)
        
        # 输出层
        z = np.dot(self.activations[-1], self.weights[-1]) + self.biases[-1]
        self.z_values.append(z)
        a = self.sigmoid(z)
        self.activations.append(a)
        
        return self.activations[-1]
    
    def compute_loss(self, y_true, y_pred):
        """计算损失函数 - 修正版本"""
        m = y_true.shape[0]
        epsilon = 1e-12
        y_pred = np.clip(y_pred, epsilon, 1. - epsilon)
        
        # 基础交叉熵损失
        base_loss = - (y_true * np.log(y_pred) + (1 - y_true) * np.log(1 - y_pred))
        
        # 应用代价敏感学习
        if self.cost_matrix is not None:
            TN_cost, FP_cost, FN_cost, TP_cost = self.cost_matrix
            
            # 为每个样本分配代价权重
            cost_weights = np.ones_like(y_true)
            
            # 真阴性 (TN): 正确识别恶性
            cost_weights[(y_true == 0) & (y_pred < 0.5)] = TN_cost
            
            # 假阳性 (FP): 良性被误判为恶性
            cost_weights[(y_true == 1) & (y_pred >= 0.5)] = FP_cost
            
            # 假阴性 (FN): 恶性被误判为良性 - 这是最危险的错误！
            cost_weights[(y_true == 0) & (y_pred >= 0.5)] = FN_cost
            
            # 真阳性 (TP): 正确识别良性
            cost_weights[(y_true == 1) & (y_pred < 0.5)] = TP_cost
            
            weighted_loss = base_loss * cost_weights
            cross_entropy = np.mean(weighted_loss)
        else:
            cross_entropy = np.mean(base_loss)
        
        # L2正则化
        reg_term = 0
        for w in self.weights:
            reg_term += np.sum(np.square(w))
        reg_term = (self.reg_lambda / (2 * m)) * reg_term
        
        return cross_entropy + reg_term
    
    def backward(self, X, y, output):
        """反向传播 - 修正版本"""
        m = X.shape[0]
        
        # 计算输出层误差
        delta = output - y
        
        # 应用代价敏感学习到误差
        if self.cost_matrix is not None:
            TN_cost, FP_cost, FN_cost, TP_cost = self.cost_matrix
            
            cost_weights = np.ones_like(y)
            
            # 根据预测和真实标签分配代价权重
            predictions = (output > 0.5).astype(int)
            
            # 假阴性 (FN): 恶性被误判为良性 - 给予最高权重
            fn_mask = (y == 0) & (predictions == 1)
            cost_weights[fn_mask] = FN_cost
            
            # 假阳性 (FP): 良性被误判为恶性
            fp_mask = (y == 1) & (predictions == 0)
            cost_weights[fp_mask] = FP_cost
            
            # 真阳性 (TP) 和真阴性 (TN) 使用较低权重
            tp_mask = (y == 1) & (predictions == 1)
            tn_mask = (y == 0) & (predictions == 0)
            cost_weights[tp_mask] = TP_cost
            cost_weights[tn_mask] = TN_cost
            
            delta = delta * cost_weights
        
        # 梯度裁剪
        delta = np.clip(delta, -5, 5)
        
        # 反向传播
        for i in range(len(self.weights) - 1, 0, -1):
            dW = np.dot(self.activations[i].T, delta) / m + (self.reg_lambda / m) * self.weights[i]
            db = np.sum(delta, axis=0, keepdims=True) / m
            
            # 梯度裁剪
            dW = np.clip(dW, -1, 1)
            
            self.weights[i] -= self.learning_rate * dW
            self.biases[i] -= self.learning_rate * db
            
            # 计算前一层的误差
            delta = np.dot(delta, self.weights[i].T) * self.relu_derivative(self.activations[i])
            delta = np.clip(delta, -1, 1)
        
        # 更新第一层
        dW = np.dot(self.activations[0].T, delta) / m + (self.reg_lambda / m) * self.weights[0]
        db = np.sum(delta, axis=0, keepdims=True) / m
        dW = np.clip(dW, -1, 1)
        
        self.weights[0] -= self.learning_rate * dW
        self.biases[0] -= self.learning_rate * db
    
    def predict(self, X, threshold=0.5):
        """预测"""
        output = self.forward(X)
        return (output > threshold).astype(int)
    
    def predict_proba(self, X):
        """预测概率"""
        return self.forward(X)

def analyze_dataset():
    """详细分析数据集"""
    data = load_breast_cancer()
    X, y = data.data, data.target
    
    print("=" * 60)
    print("数据集详细分析")
    print("=" * 60)
    
    # 类别分布
    malignant_count = np.sum(y == 0)  # 恶性
    benign_count = np.sum(y == 1)     # 良性
    
    print(f"总样本数: {len(y)}")
    print(f"恶性样本数 (标签 0): {malignant_count}")
    print(f"良性样本数 (标签 1): {benign_count}")
    print(f"恶性样本比例: {malignant_count/len(y):.3f}")
    print(f"良性样本比例: {benign_count/len(y):.3f}")
    
    # 确定少数类和多数类
    if malignant_count < benign_count:
        minority_class = 0  # 恶性是少数类
        majority_class = 1  # 良性是多数类
        print(f"少数类: 恶性 (标签 0), {malignant_count}个样本")
        print(f"多数类: 良性 (标签 1), {benign_count}个样本")
    else:
        minority_class = 1  # 良性是少数类
        majority_class = 0  # 恶性是多数类
        print(f"少数类: 良性 (标签 1), {benign_count}个样本")
        print(f"多数类: 恶性 (标签 0), {malignant_count}个样本")
    
    print(f"不平衡比例: {max(malignant_count, benign_count)/min(malignant_count, benign_count):.2f}:1")
    
    return minority_class, majority_class, malignant_count, benign_count

def calculate_appropriate_cost_matrix(minority_class, malignant_count, benign_count):
    """计算合适的代价矩阵"""
    print("\n" + "=" * 60)
    print("代价矩阵设置")
    print("=" * 60)
    
    # 基础代价
    base_cost = 1.0
    
    if minority_class == 0:  # 恶性是少数类
        print("情况: 恶性是少数类，需要特别关注")
        
        # 代价矩阵: [TN_cost, FP_cost, FN_cost, TP_cost]
        # TN: 正确识别恶性 (恶性→恶性) - 基础代价
        # FP: 良性误判为恶性 (良性→恶性) - 中等代价
        # FN: 恶性误判为良性 (恶性→良性) - 最高代价！这是最危险的
        # TP: 正确识别良性 (良性→良性) - 基础代价
        
        TN_cost = base_cost                    # 真阴性: 正确识别恶性
        FP_cost = base_cost * 1.5              # 假阳性: 良性→恶性 (中等代价)
        FN_cost = base_cost * 3.0              # 假阴性: 恶性→良性 (最高代价！)
        TP_cost = base_cost                    # 真阳性: 正确识别良性
        
        print("恶性误判为良性是最危险的错误！")
        
    else:  # 良性是少数类
        print("情况: 良性是少数类")
        
        TN_cost = base_cost                    # 真阴性: 正确识别恶性
        FP_cost = base_cost * 2.0              # 假阳性: 良性→恶性 (较高代价)
        FN_cost = base_cost * 1.5              # 假阴性: 恶性→良性 (中等代价)
        TP_cost = base_cost                    # 真阳性: 正确识别良性
    
    cost_matrix = [TN_cost, FP_cost, FN_cost, TP_cost]
    
    print(f"代价矩阵: TN={TN_cost}, FP={FP_cost}, FN={FN_cost}, TP={TP_cost}")
    print("TN: 恶性正确识别, FP: 良性误判为恶性, FN: 恶性误判为良性, TP: 良性正确识别")
    print(f"最关注的错误: {'恶性误判为良性 (FN)' if minority_class == 0 else '良性误判为恶性 (FP)'}")
    
    return cost_matrix

def load_and_preprocess_data():
    """加载和预处理数据"""
    data = load_breast_cancer()
    X, y = data.data, data.target
    
    # 标准化
    scaler = StandardScaler()
    X = scaler.fit_transform(X)
    
    # 分割数据集
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y
    )
    
    X_train, X_val, y_train, y_val = train_test_split(
        X_train, y_train, test_size=0.1, random_state=42, stratify=y_train
    )
    
    # 转换为列向量
    y_train = y_train.reshape(-1, 1)
    y_val = y_val.reshape(-1, 1)
    y_test = y_test.reshape(-1, 1)
    
    print(f"\n数据分割:")
    print(f"训练集: {X_train.shape}")
    print(f"验证集: {X_val.shape}")
    print(f"测试集: {X_test.shape}")
    
    return X_train, X_val, X_test, y_train, y_val, y_test, scaler, data

def train_model_safely(model, X_train, y_train, X_val, y_val, epochs=500, batch_size=32):
    """安全的模型训练"""
    train_losses = []
    val_losses = []
    val_accuracies = []
    
    n_batches = int(np.ceil(X_train.shape[0] / batch_size))
    
    print("\n开始训练...")
    start_time = time.time()
    
    for epoch in range(epochs):
        # 打乱数据
        indices = np.random.permutation(X_train.shape[0])
        X_shuffled = X_train[indices]
        y_shuffled = y_train[indices]
        
        epoch_loss = 0
        
        # 小批量训练
        for i in range(n_batches):
            start_idx = i * batch_size
            end_idx = min((i + 1) * batch_size, X_train.shape[0])
            
            X_batch = X_shuffled[start_idx:end_idx]
            y_batch = y_shuffled[start_idx:end_idx]
            
            output = model.forward(X_batch)
            batch_loss = model.compute_loss(y_batch, output)
            epoch_loss += batch_loss
            
            model.backward(X_batch, y_batch, output)
        
        # 验证集评估
        val_output = model.forward(X_val)
        val_loss = model.compute_loss(y_val, val_output)
        val_pred = model.predict(X_val)
        val_acc = accuracy_score(y_val, val_pred)
        
        train_losses.append(epoch_loss / n_batches)
        val_losses.append(val_loss)
        val_accuracies.append(val_acc)
        
        if epoch % 50 == 0:
            print(f"Epoch {epoch}: Train Loss = {train_losses[-1]:.4f}, "
                  f"Val Loss = {val_losses[-1]:.4f}, Val Acc = {val_acc:.4f}")
    
    end_time = time.time()
    print(f"训练完成! 耗时: {end_time - start_time:.2f}秒")
    
    return {
        'train_losses': train_losses,
        'val_losses': val_losses,
        'val_accuracies': val_accuracies
    }

def comprehensive_evaluation(model, X_test, y_test, minority_class):
    """综合评估模型"""
    y_pred = model.predict(X_test)
    y_pred_proba = model.predict_proba(X_test)
    
    # 计算指标
    accuracy = accuracy_score(y_test, y_pred)
    cm = confusion_matrix(y_test, y_pred)
    
    if cm.shape[0] == 2:
        tn, fp, fn, tp = cm.ravel()
        
        # 敏感性 (召回率) - 对良性样本的识别能力
        sensitivity = tp / (tp + fn) if (tp + fn) > 0 else 0
        
        # 特异性 - 对恶性样本的识别能力
        specificity = tn / (tn + fp) if (tn + fp) > 0 else 0
        
        # 精确度
        precision = tp / (tp + fp) if (tp + fp) > 0 else 0
        
        # F1分数
        f1 = 2 * precision * sensitivity / (precision + sensitivity) if (precision + sensitivity) > 0 else 0
        
        # 平衡准确率
        balanced_accuracy = (sensitivity + specificity) / 2
        
        # G-mean (几何平均)
        gmean = np.sqrt(sensitivity * specificity) if (sensitivity > 0 and specificity > 0) else 0
    else:
        sensitivity = specificity = precision = f1 = balanced_accuracy = gmean = 0
    
    print("=" * 60)
    print("模型评估结果")
    print("=" * 60)
    print(f"准确率 (Accuracy): {accuracy:.4f}")
    print(f"平衡准确率 (Balanced Accuracy): {balanced_accuracy:.4f}")
    print(f"G-Mean: {gmean:.4f}")
    print(f"敏感性 (Sensitivity): {sensitivity:.4f} - 识别良性的能力")
    print(f"特异性 (Specificity): {specificity:.4f} - 识别恶性的能力")
    print(f"精确度 (Precision): {precision:.4f}")
    print(f"F1分数: {f1:.4f}")
    
    print(f"\n混淆矩阵:")
    print(f"          预测恶性   预测良性")
    print(f"实际恶性    {cm[0,0]:4d} (TN)    {cm[0,1]:4d} (FN)")
    print(f"实际良性    {cm[1,0]:4d} (FP)    {cm[1,1]:4d} (TP)")
    
    # 特别关注关键错误
    if minority_class == 0:  # 恶性是少数类
        fn_rate = cm[0,1] / (cm[0,0] + cm[0,1]) if (cm[0,0] + cm[0,1]) > 0 else 0
        print(f"\n关键指标:")
        print(f"恶性误判为良性的比例: {fn_rate:.4f} (应尽可能低)")
        print(f"恶性样本正确识别率: {specificity:.4f} (应尽可能高)")
    else:  # 良性是少数类
        fp_rate = cm[1,0] / (cm[1,0] + cm[1,1]) if (cm[1,0] + cm[1,1]) > 0 else 0
        print(f"\n关键指标:")
        print(f"良性误判为恶性的比例: {fp_rate:.4f} (应尽可能低)")
        print(f"良性样本正确识别率: {sensitivity:.4f} (应尽可能高)")
    
    # ROC曲线
    fpr, tpr, _ = roc_curve(y_test, y_pred_proba)
    roc_auc = auc(fpr, tpr)
    
    plt.figure(figsize=(10, 4))
    
    plt.subplot(1, 2, 1)
    plt.plot(fpr, tpr, color='darkorange', lw=2, label=f'ROC曲线 (AUC = {roc_auc:.4f})')
    plt.plot([0, 1], [0, 1], color='navy', lw=2, linestyle='--')
    plt.xlim([0.0, 1.0])
    plt.ylim([0.0, 1.05])
    plt.xlabel('假正率 (FPR)')
    plt.ylabel('真正率 (TPR)')
    plt.title('ROC曲线')
    plt.legend(loc="lower right")
    plt.grid(True)
    
    # 预测概率分布
    plt.subplot(1, 2, 2)
    malignant_probs = y_pred_proba[y_test.flatten() == 0]
    benign_probs = y_pred_proba[y_test.flatten() == 1]
    
    plt.hist(malignant_probs, alpha=0.7, bins=20, label='恶性样本', color='red')
    plt.hist(benign_probs, alpha=0.7, bins=20, label='良性样本', color='green')
    plt.xlabel('预测概率')
    plt.ylabel('频数')
    plt.title('预测概率分布')
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    plt.show()
    
    print(f"\nAUC: {roc_auc:.4f}")
    
    return accuracy, balanced_accuracy, sensitivity, specificity, precision, f1, roc_auc

def main():
    """主函数"""
    # 分析数据集
    minority_class, majority_class, malignant_count, benign_count = analyze_dataset()
    
    # 加载数据
    X_train, X_val, X_test, y_train, y_val, y_test, scaler, data = load_and_preprocess_data()
    
    # 计算合适的代价矩阵
    cost_matrix = calculate_appropriate_cost_matrix(minority_class, malignant_count, benign_count)
    
    # 创建修正的神经网络
    layers = [X_train.shape[1], 16, 8, 1]
    nn = CorrectedBPNeuralNetwork(
        layers, 
        learning_rate=0.01,    # 适当的学习率
        reg_lambda=0.001,      # 适度的正则化
        cost_matrix=cost_matrix  # 使用代价矩阵而不是类别权重
    )
    
    print(f"\n神经网络配置:")
    print(f"网络结构: {layers}")
    print(f"学习率: {nn.learning_rate}")
    print(f"正则化参数: {nn.reg_lambda}")
    
    # 训练模型
    history = train_model_safely(
        nn, X_train, y_train, X_val, y_val,
        epochs=300, 
        batch_size=32
    )
    
    # 综合评估
    print("\n最终模型评估:")
    results = comprehensive_evaluation(nn, X_test, y_test, minority_class)
    
    # 绘制训练历史
    plt.figure(figsize=(12, 4))
    
    plt.subplot(1, 2, 1)
    plt.plot(history['train_losses'], label='训练损失')
    plt.plot(history['val_losses'], label='验证损失')
    plt.title('训练和验证损失')
    plt.xlabel('Epoch')
    plt.ylabel('Loss')
    plt.legend()
    plt.grid(True)
    
    plt.subplot(1, 2, 2)
    plt.plot(history['val_accuracies'], label='验证准确率', color='orange')
    plt.title('验证准确率')
    plt.xlabel('Epoch')
    plt.ylabel('Accuracy')
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    plt.show()
    
    return nn, results, minority_class

if __name__ == "__main__":
    model, results, minority_class = main()