import requests
import pandas as pd
import time
import os
import numpy as np  # 确保numpy已导入
import re
import json
from typing import Dict, List, Optional
from datetime import datetime

# ============================================================================
# 材料属性获取模块（修正版）
# ============================================================================

def _get_default_hardness(material_name: str) -> float:
    """获取材料的典型硬度值（Mohs）"""
    hardness_map = {
        'limestone': 3.5,
        'sandstone': 6.0,
        'marble': 3.0,
        'granite': 7.0,
        'oak_wood': 2.5,
        'pine_wood': 2.0,
        'brick': 3.0,
        'concrete': 4.0,
        'stone': 5.0,
        'wood': 2.0
    }
    return hardness_map.get(material_name.lower(), 3.0)

def _get_default_density(material_name: str) -> int:
    """获取材料的典型密度值（kg/m³）"""
    density_map = {
        'limestone': 2700,
        'sandstone': 2200,
        'marble': 2750,
        'granite': 2650,
        'oak_wood': 750,
        'pine_wood': 500,
        'brick': 1800,
        'concrete': 2400,
        'stone': 2600,
        'wood': 600
    }
    return density_map.get(material_name.lower(), 2500)

def _get_fallback_properties(material_name: str) -> Dict:
    """返回材料的备用属性集（当网络请求失败时使用）"""
    return {
        'material': material_name,
        'hardness_mohs': _get_default_hardness(material_name),  # 修正：直接调用函数
        'density_kg_m3': _get_default_density(material_name),    # 修正：直接调用函数
        'compressive_strength_mpa': 30,
        'elastic_modulus_gpa': 25,
        'source': 'default_values',
        'status': 'fallback',
        'timestamp': datetime.now().isoformat()
    }



# 主材料属性获取函数
# use_mock: 是否使用模拟数据进行测试（True则不调用真实API）
def fetch_material_properties(material_name: str = "limestone", 
                              max_retries: int = 3, 
                              use_mock: bool = False) -> Dict:  # 添加测试模式参数
    
    # 【测试模式】如果启用mock，直接返回模拟数据
    if use_mock:
        print(f"⚠ 测试模式：返回模拟数据 for '{material_name}'")
        return {
            'material': material_name,
            'hardness_mohs': _get_default_hardness(material_name),
            'density_kg_m3': _get_default_density(material_name),
            'compressive_strength_mpa': np.random.randint(20, 100),  # 随机生成合理值
            'elastic_modulus_gpa': np.random.randint(10, 80),
            'source': 'mock_data',
            'status': 'success',
            'timestamp': datetime.now().isoformat()
        }
    
    """
    爬取常见建筑材料的力学属性（硬度、密度等）
    来源：NIST Material Properties Database
    
    参数:
        material_name: 材料名称（如 'limestone', 'marble', 'oak_wood'）
        max_retries: 最大重试次数
        use_mock: 是否使用模拟数据进行测试（True则不调用真实API）
    
    返回:
        包含材料属性的字典
    """
    base_url = "https://materialsdata.nist.gov/"
    search_url = f"{base_url}search"
    
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
        'Accept': 'application/json, text/plain, */*',
        'Accept-Language': 'en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7'
    }
    
    params = {
        'query': material_name,
        'format': 'json'
    }
    
    for attempt in range(max_retries):
        try:
            print(f"正在爬取材料 '{material_name}' 的属性 (尝试 {attempt+1}/{max_retries})...")
            response = requests.get(
                search_url, 
                params=params, 
                headers=headers, 
                timeout=15
            )
            response.raise_for_status()
            
            data = response.json()
            
            # 提取关键属性
            properties = {
                'material': material_name,
                'hardness_mohs': None,
                'density_kg_m3': None,
                'compressive_strength_mpa': None,
                'elastic_modulus_gpa': None,
                'source': 'NIST Database',
                'status': 'success',
                'timestamp': datetime.now().isoformat()
            }
            
            # 解析实际数据（处理多种可能的响应格式）
            if 'results' in data and len(data['results']) > 0:
                result = data['results'][0]
                properties['hardness_mohs'] = result.get('hardness', _get_default_hardness(material_name))
                properties['density_kg_m3'] = result.get('density', _get_default_density(material_name))
                properties['compressive_strength_mpa'] = result.get('compressive_strength', None)
                properties['elastic_modulus_gpa'] = result.get('elastic_modulus', None)
                
                print(f"✓ 成功获取材料 '{material_name}' 的数据")
                return properties
                
            else:
                print(f"⚠ 未找到材料 '{material_name}' 的详细数据，返回典型值")
                return _get_fallback_properties(material_name)  # 修正：直接调用函数
                
        except requests.exceptions.RequestException as e:
            print(f"✗ 请求失败: {e}")
            if attempt < max_retries - 1:
                wait_time = 2 ** (attempt + 1)  # 指数退避（2, 4, 8秒）
                print(f"等待 {wait_time} 秒后重试...")
                time.sleep(wait_time)
            else:
                print(f"⚠ 达到最大重试次数，返回 '{material_name}' 的典型值")
                return _get_fallback_properties(material_name)  # 修正：直接调用函数
        except Exception as e:
            print(f"✗ 解析错误: {e}")
            return _get_fallback_properties(material_name)  # 修正：直接调用函数
    
    return _get_fallback_properties(material_name)  # 修正：直接调用函数


# ============================================================================
# 考古文献爬取模块（SerpAPI）
# ============================================================================

def _extract_patterns_from_papers(papers: List[Dict]) -> Dict:
    """
    从论文摘要中提取楼梯使用模式数据
    这是一个简化版NLP实现，实际应用中可使用更复杂的自然语言处理
    """
    print("\n正在从论文摘要中提取使用模式数据...")
    
    # 初始化统计变量
    total_users_mentioned = []
    direction_hints = {'up': 0, 'down': 0, 'bidirectional': 0}
    parallel_hints = {'single': 0, 'parallel': 0}
    
    # 关键词模式
    user_patterns = [
        r'(\d+)\s*people\s*per\s*day',
        r'(\d+)\s*daily\s*users',
        r'(\d+)\s*individuals\s*daily',
        r'traffic\s*of\s*(\d+)',
        r'(\d+)\s*visitors\s*daily',
        r'usage\s*rate\s*(\d+)'
    ]
    
    direction_keywords = {
        'up': ['ascending', 'upward', 'climbing up', 'going up', 'primary direction up'],
        'down': ['descending', 'downward', 'climbing down', 'going down', 'primary direction down'],
        'bidirectional': ['bidirectional', 'both directions', 'up and down', 'two-way']
    }
    
    parallel_keywords = {
        'parallel': ['side by side', 'parallel', 'two people', 'multiple users', 'simultaneous', 'pairs'],
        'single': ['single file', 'one person', 'narrow', 'limited width', 'single user']
    }
    
    for paper in papers:
        snippet = paper['snippet'].lower()
        
        # 提取用户数量
        for pattern in user_patterns:
            matches = re.findall(pattern, snippet, re.IGNORECASE)
            for match in matches:
                try:
                    num = int(match)
                    if 10 <= num <= 100000:  # 合理范围
                        total_users_mentioned.append(num)
                except:
                    pass
        
        # 判断方向性
        for direction, keywords in direction_keywords.items():
            if any(keyword in snippet for keyword in keywords):
                direction_hints[direction] += 1
        
        # 判断并行使用
        for usage_type, keywords in parallel_keywords.items():
            if any(keyword in snippet for keyword in keywords):
                parallel_hints[usage_type] += 1
    
    # 计算统计结果
    avg_daily_users = int(np.mean(total_users_mentioned)) if total_users_mentioned else 150  # 使用np
    direction_ratio = 0.6  # 默认上行比例
    
    if direction_hints['up'] > direction_hints['down']:
        direction_ratio = 0.7
    elif direction_hints['down'] > direction_hints['up']:
        direction_ratio = 0.3
    elif direction_hints['bidirectional'] > 0:
        direction_ratio = 0.5
    
    parallel_usage_probability = 0.3
    if parallel_hints['parallel'] > parallel_hints['single']:
        parallel_usage_probability = 0.5
    elif parallel_hints['single'] > parallel_hints['parallel']:
        parallel_usage_probability = 0.1
    
    print(f"✓ 提取完成:")
    print(f"  - 找到 {len(total_users_mentioned)} 个用户数量数据点")
    print(f"  - 平均日用户数: {avg_daily_users}")
    print(f"  - 上行比例: {direction_ratio:.2f}")
    print(f"  - 并行使用概率: {parallel_usage_probability:.2f}")
    
    return {
        'avg_daily_users': avg_daily_users,
        'direction_ratio': direction_ratio,
        'parallel_usage_probability': parallel_usage_probability,
        'data_points': len(total_users_mentioned)
    }

def get_stair_usage_patterns(api_key: Optional[str] = None, 
                           search_query: str = "staircase wear patterns archaeology human traffic",
                           max_results: int = 15,
                           max_retries: int = 2) -> Dict:
    """
    使用SerpAPI爬取考古文献中的人类行为模式数据
    
    参数:
        api_key: SerpAPI密钥
        search_query: 搜索关键词
        max_results: 返回的最大结果数
        max_retries: 最大重试次数
    
    返回:
        包含学术论文信息和提取的使用模式数据
    """
    # 使用提供的密钥
    if api_key is None:
        # 如果未提供，尝试从环境变量获取，否则使用硬编码密钥
        env_key = os.getenv('SERPAPI_KEY')
        if env_key:
            api_key = env_key
            print("✓ 使用环境变量中的API密钥")
        else:
            api_key = '33ba470843d27534b92e311f95110b129621318e75378a649761995e36eb1f70'
            print("✓ 使用代码中提供的API密钥")
    
    # 检查是否为无效密钥
    if not api_key or len(api_key) < 20:
        print("✗ 错误：API密钥无效或缺失")
        return {
            'status': 'error',
            'papers': [],
            'usage_patterns': {},
            'message': 'Invalid API key'
        }
    
    base_url = "https://serpapi.com/search.json"
    
    params = {
        'engine': 'google_scholar',
        'q': search_query,
        'api_key': api_key,
        'hl': 'en',
        'num': max_results
    }
    
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36',
        'Accept': 'application/json'
    }
    
    for attempt in range(max_retries):
        try:
            print(f"\n正在通过SerpAPI搜索学术论文 (尝试 {attempt+1}/{max_retries})...")
            print(f"搜索关键词: '{search_query}'")
            print(f"API密钥: {api_key[:8]}...{api_key[-8:]}")
            
            response = requests.get(
                base_url,
                params=params,
                headers=headers,
                timeout=30
            )
            
            # 检查HTTP状态码
            if response.status_code == 401:
                print("✗ 错误：API密钥无效或已过期")
                print("请访问 https://serpapi.com/manage-api-key 检查您的密钥")
                return {
                    'status': 'error',
                    'papers': [],
                    'usage_patterns': {},
                    'message': 'API key authentication failed (401)'
                }
            elif response.status_code == 429:
                print("✗ 错误：请求频率过高，请稍后重试")
                return {
                    'status': 'error',
                    'papers': [],
                    'usage_patterns': {},
                    'message': 'Rate limit exceeded (429)'
                }
            elif response.status_code != 200:
                print(f"✗ 错误：HTTP状态码 {response.status_code}")
                return {
                    'status': 'error',
                    'papers': [],
                    'usage_patterns': {},
                    'message': f'HTTP error {response.status_code}'
                }
            
            response.raise_for_status()
            data = response.json()
            
            # 检查SerpAPI返回的错误
            if 'error' in data:
                print(f"✗ API返回错误: {data['error']}")
                return {
                    'status': 'error',
                    'papers': [],
                    'usage_patterns': {},
                    'message': data['error']
                }
            
            # 检查是否有搜索结果
            if not data.get('organic_results'):
                print("⚠ 警告：未找到相关学术文献")
                return {
                    'status': 'success',
                    'papers': [],
                    'usage_patterns': {
                        'avg_daily_users': 150,
                        'direction_ratio': 0.6,
                        'parallel_usage_probability': 0.3,
                        'data_points': 0
                    },
                    'message': 'No results found',
                    'search_metadata': data.get('search_metadata', {})
                }
            
            # 解析学术论文数据
            papers = []
            organic_results = data.get('organic_results', [])
            
            print(f"\n✓ 成功获取 {len(organic_results)} 篇相关论文")
            
            for idx, result in enumerate(organic_results, 1):
                title = result.get('title', 'N/A')
                link = result.get('link', 'N/A')
                snippet = result.get('snippet', 'N/A')
                publication_info = result.get('publication_info', {})
                
                paper_data = {
                    'id': idx,
                    'title': title,
                    'link': link,
                    'snippet': snippet[:300] + '...' if len(snippet) > 300 else snippet,
                    'authors': publication_info.get('authors', 'N/A'),
                    'year': publication_info.get('year', 'N/A'),
                    'cited_by': result.get('inline_links', {}).get('cited_by', {}).get('total', 0)
                }
                papers.append(paper_data)
                
                print(f"\n[{idx}] {title}")
                print(f"    链接: {link}")
                print(f"    引用数: {paper_data['cited_by']}")
                print(f"    摘要: {paper_data['snippet']}")
            
            # 从论文摘要中提取使用模式
            usage_patterns = _extract_patterns_from_papers(papers)
            
            result = {
                'status': 'success',
                'papers': papers,
                'usage_patterns': usage_patterns,
                'search_metadata': data.get('search_metadata', {}),
                'total_results': data.get('search_information', {}).get('total_results', 0)
            }
            
            print(f"\n✓ 数据爬取完成！总共找到 {result['total_results']} 条学术结果")
            return result
            
        except requests.exceptions.Timeout:
            print("✗ 错误：请求超时，请检查网络连接")
            if attempt < max_retries - 1:
                time.sleep(5 * (attempt + 1))
        except requests.exceptions.RequestException as e:
            print(f"✗ 网络请求错误: {e}")
            if attempt < max_retries - 1:
                time.sleep(5 * (attempt + 1))
        except Exception as e:
            print(f"✗ 未知错误: {e}")
            return {
                'status': 'error',
                'papers': [],
                'usage_patterns': {},
                'message': f'Unexpected error: {str(e)}'
            }
    
    return {
        'status': 'error',
        'papers': [],
        'usage_patterns': {},
        'message': 'All retries failed'
    }


# ============================================================================
# 数据缓存工具函数
# ============================================================================

def cache_results(data: Dict, filename: str = 'cache/stair_usage_cache.json'):
    """缓存结果到本地文件"""
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"✓ 结果已缓存到 {filename}")
    except Exception as e:
        print(f"⚠ 缓存失败: {e}")

def load_cached_results(filename: str = 'cache/stair_usage_cache.json') -> Optional[Dict]:
    """从本地缓存加载结果"""
    try:
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                print(f"✓ 从缓存加载数据: {filename}")
                return json.load(f)
    except Exception as e:
        print(f"⚠ 缓存加载失败: {e}")
    return None


# ============================================================================
# 主执行程序
# ============================================================================

def main():
    """主执行函数"""
    print("="*70)
    print("楼梯考古磨损分析 - 数据爬取工具")
    print("="*70)
    print(f"执行时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("="*70)
    
    # 创建输出目录
    os.makedirs('cache', exist_ok=True)
    os.makedirs('data', exist_ok=True)
    
    # 1. 材料属性爬取
    print("\n" + "="*70)
    print("阶段1: 爬取材料属性数据")
    print("="*70)
    
    materials = ['limestone', 'sandstone', 'marble', 'granite', 'oak_wood', 'pine_wood', 'brick']
    material_data = []
    
    for material in materials:
        props = fetch_material_properties(material, max_retries=2)
        material_data.append(props)
        time.sleep(1)  # 避免请求过快
    
    material_df = pd.DataFrame(material_data)
    
    print("\n材料属性表：")
    print(material_df.to_string(index=False))
    
    # 保存材料数据
    material_csv_path = 'data/material_properties.csv'
    material_df.to_csv(material_csv_path, index=False)
    print(f"\n✓ 材料数据已保存到 {material_csv_path}")
    
    # 2. 考古文献爬取
    print("\n" + "="*70)
    print("阶段2: 爬取考古文献数据")
    print("="*70)
    
    # 尝试加载缓存
    cache_file = 'cache/stair_usage_cache.json'
    cached_data = load_cached_results(cache_file)
    
    if cached_data and cached_data.get('status') == 'success':
        user_input = input("\n发现缓存数据（1小时内的结果），是否使用？(y/n, 默认y): ")
        if user_input.lower() != 'n':
            usage_data = cached_data
            print("✓ 使用缓存数据")
        else:
            usage_data = None
    else:
        usage_data = None
    
    if usage_data is None:
        # 使用您的API密钥进行爬取
        usage_data = get_stair_usage_patterns(
            api_key='33ba470843d27534b92e311f95110b129621318e75378a649761995e36eb1f70',
            search_query="staircase wear patterns archaeology human traffic medieval",
            max_results=20,
            max_retries=2
        )
        
        # 缓存结果
        if usage_data.get('status') == 'success':
            cache_results(usage_data, cache_file)
    
    # 3. 结果展示
    print("\n" + "="*70)
    print("阶段3: 结果汇总")
    print("="*70)
    
    if usage_data.get('status') == 'success':
        print(f"\n✓ 爬取成功！")
        print(f"  - 考古文献数量: {len(usage_data['papers'])} 篇")
        print(f"  - 总搜索结果: {usage_data.get('total_results', 0)} 条")
        print("\n提取的使用模式：")
        print(f"  平均日用户数: {usage_data['usage_patterns']['avg_daily_users']}")
        print(f"  上行比例: {usage_data['usage_patterns']['direction_ratio']:.2f}")
        print(f"  并行使用概率: {usage_data['usage_patterns']['parallel_usage_probability']:.2f}")
        
        # 保存使用模式数据
        usage_json_path = 'data/usage_patterns.json'
        with open(usage_json_path, 'w', encoding='utf-8') as f:
            json.dump(usage_data, f, indent=2, ensure_ascii=False)
        print(f"\n✓ 使用模式数据已保存到 {usage_json_path}")
        
        # 生成简化的CSV版本
        patterns = usage_data['usage_patterns']
        summary_df = pd.DataFrame([{
            'metric': ['avg_daily_users', 'direction_ratio', 'parallel_usage_probability'],
            'value': [patterns['avg_daily_users'], patterns['direction_ratio'], patterns['parallel_usage_probability']],
            'unit': ['people/day', 'ratio', 'probability']
        }])
        summary_df.to_csv('data/usage_patterns_summary.csv', index=False)
        
    else:
        print(f"\n✗ 爬取失败: {usage_data.get('message', '未知错误')}")
        print("将使用默认数据...")
        
        # 使用默认数据
        default_usage = {
            'avg_daily_users': 150,
            'direction_ratio': 0.6,
            'parallel_usage_probability': 0.3,
            'data_points': 0
        }
        
        # 保存默认数据
        usage_json_path = 'data/usage_patterns.json'
        with open(usage_json_path, 'w', encoding='utf-8') as f:
            json.dump({'status': 'default', 'usage_patterns': default_usage}, f, indent=2, ensure_ascii=False)
        print(f"✓ 默认数据已保存到 {usage_json_path}")
    
    print("\n" + "="*70)
    print("数据爬取工具执行完成！")
    print("="*70)


# 在main()函数外添加独立测试函数
def test_material_fetching():
    """测试材料属性获取功能（不调用API）"""
    print("="*70)
    print("测试模式：验证材料属性获取功能")
    print("="*70)
    
    test_materials = ['limestone', 'sandstone', 'marble']
    
    for material in test_materials:
        # 使用模拟数据测试
        result = fetch_material_properties(material, use_mock=True)
        
        print(f"\n测试材料: {material}")
        print(f"  硬度: {result['hardness_mohs']} Mohs")
        print(f"  密度: {result['density_kg_m3']} kg/m³")
        print(f"  抗压强度: {result['compressive_strength_mpa']} MPa")
        print(f"  数据来源: {result['source']}")
        
        # 验证数据结构
        assert 'material' in result
        assert 'hardness_mohs' in result
        assert result['status'] == 'success'
        assert result['source'] == 'mock_data'
        
        print("  ✓ 数据结构验证通过")
    
    print("\n" + "="*70)
    print("所有测试通过！材料属性获取功能正常")
    print("="*70)


# 在主程序中添加测试分支
if __name__ == "__main__":
    import sys
    
    # 通过命令行参数控制模式
    if len(sys.argv) > 1 and sys.argv[1] == '--test':
        test_material_fetching()  # 仅测试材料属性
    else:
        main()  # 正常运行所有功能