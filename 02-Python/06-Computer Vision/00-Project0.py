import cv2
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import os
import warnings
from pylab import mpl
mpl.rcParams['font.sans-serif'] = ['STZhongsong']    # 指定默认字体：解决plot不能显示中文问题
mpl.rcParams['axes.unicode_minus'] = False           # 解决保存图像是负号'-'显示为方块的问题
warnings.filterwarnings('ignore')

# 设置文件夹路径（注意使用原始字符串或双反斜杠）
folder_path = r"E:\06-ZYZ-CV"
# 五张图片的文件名列表
file_names = [
    "low resolution.jpg",
    "Switzerland - Lake Brienz.jpg",
    "Bolivian church.jpg",
    "Snowy mountain.jpg",
    "gray.jpg"
]

# 3) 使用Pillow和OpenCV读取图像，并通过Matplotlib显示
print("开始第3步：对比Pillow和OpenCV读取的图像...")
for i, fname in enumerate(file_names):
    file_path = os.path.join(folder_path, fname)
    if not os.path.exists(file_path):
        print(f"文件不存在: {file_path}")
        continue

    # Pillow读取（直接得到PIL Image对象）
    pil_img = Image.open(file_path)
    # 将PIL Image转为numpy数组（RGB顺序）
    pil_arr = np.array(pil_img)

    # OpenCV读取（得到numpy数组，BGR顺序）
    cv_img = cv2.imread(file_path)
    if cv_img is None:
        print(f"OpenCV无法读取: {file_path}")
        continue
    # OpenCV默认BGR，转换为RGB以便显示
    cv_img_rgb = cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB)

    # 使用Matplotlib显示对比
    plt.figure(figsize=(10, 5))
    plt.subplot(1, 2, 1)
    plt.imshow(pil_arr)
    plt.title(f'Pillow读取 (RGB)\n尺寸: {pil_arr.shape[1]}x{pil_arr.shape[0]}')
    plt.axis('off')

    plt.subplot(1, 2, 2)
    plt.imshow(cv_img_rgb)
    plt.title(f'OpenCV读取 (BGR转RGB)\n尺寸: {cv_img.shape[1]}x{cv_img.shape[0]}')
    plt.axis('off')

    plt.suptitle(f'图片: {fname}')
    plt.show()


# 4) 对图像进行放大3倍和沿x方向平移2像素处理（使用OpenCV）
print("\n开始第4步：图像放大3倍 + x方向平移2像素...")
# 选择第一张图片作为示例
sample_file = file_names[0]
sample_path = os.path.join(folder_path, sample_file)
if os.path.exists(sample_path):
    # 读取图像（OpenCV格式，BGR）
    img = cv2.imread(sample_path)
    if img is not None:
        h, w = img.shape[:2]
        print(f"原图尺寸: {w} x {h}")

        # (a) 放大3倍
        enlarged = cv2.resize(img, None, fx=3, fy=3, interpolation=cv2.INTER_LINEAR)
        h_en, w_en = enlarged.shape[:2]
        print(f"放大3倍后尺寸: {w_en} x {h_en}")

        # (b) 沿x方向平移2像素
        # 定义平移矩阵 [1,0,tx; 0,1,ty]
        M = np.float32([[1, 0, 2], [0, 1, 0]])
        # 保持输出图像尺寸与原图相同（平移后超出的部分被裁剪，空白处填充为黑色）
        translated = cv2.warpAffine(img, M, (w, h))
        # 如果需要保持尺寸不变但平移后内容可能超出边界，这里采用保持尺寸的方式

        # 显示原图和处理结果
        plt.figure(figsize=(15, 5))
        plt.subplot(1, 2, 1)
        plt.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
        plt.title('原始图像')
        plt.axis('off')

        plt.subplot(1, 2, 2)
        # 这里我是分两次获得了放大和平移的图片
        # plt.imshow(cv2.cvtColor(enlarged, cv2.COLOR_BGR2RGB))
        # plt.title('放大3倍')
        # plt.axis('off')

        plt.imshow(cv2.cvtColor(translated, cv2.COLOR_BGR2RGB))
        plt.title('x方向平移2像素')
        plt.axis('off')

        plt.suptitle(f'处理效果: {sample_file}')
        plt.show()

        # 可选的保存结果
        # cv2.imwrite("enlarged.jpg", enlarged)
        # cv2.imwrite("translated.jpg", translated)
    else:
        print("无法读取样本图片")
else:
    print(f"样本图片不存在: {sample_path}")

print("所有操作完成。")