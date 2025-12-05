from dotenv import load_dotenv
import os
load_dotenv() # 加载 .env 文件中的环境变量
api_key = os.getenv("SERP_API_KEY")
if not api_key:
   raise ValueError("未设置 API_KEY 环境变量")
print(f"API Key: {api_key}")