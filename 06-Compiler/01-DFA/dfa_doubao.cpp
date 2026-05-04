#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

using namespace std;

// DFA结构体：存储五元组
struct DFA
{
    set<char> sigma;                 // 字符集
    set<int> states;                 // 状态集
    int start;                       // 开始状态
    set<int> accept;                 // 接受状态集
    map<pair<int, char>, int> trans; // 状态转换表 (当前状态, 输入字符) -> 下一状态
};

DFA dfa; // 全局DFA对象

// 函数声明
bool readDFAFromFile(const string &filename);              // 从文件读取DFA
bool checkDFA();                                           // 检查DFA合法性
void generateAllStrings(int maxLen);                       // 生成所有长度<=N的合法字符串
void dfsGenerate(int curState, string curStr, int maxLen); // 深度优先生成字符串
bool judgeString(const string &str);                       // 判定字符串是否合法
void simulateProcess(const string &str);                   // 模拟DFA识别过程
string randomString(int len);                              // 随机生成字符串
void showMenu();                                           // 显示菜单

int main()
{
    // 初始化随机数种子
    srand(time(0));

    // 1. 读取DFA文件
    string filename;
    cout << "请输入DFA五元组文件路径(如dfa.txt): ";
    cin >> filename;
    if (!readDFAFromFile(filename))
    {
        cout << "文件读取失败！程序退出" << endl;
        return 1;
    }
    cout << "DFA读取成功！" << endl;

    // 2. 检查DFA合法性
    if (!checkDFA())
    {
        cout << "DFA不合法！程序退出" << endl;
        return 1;
    }
    cout << "DFA合法性检查通过！" << endl;

    // 3. 菜单功能
    while (true)
    {
        showMenu();
        int choice;
        cin >> choice;
        if (choice == 1)
        {
            // 生成所有长度<=N的合法字符串
            int N;
            cout << "请输入字符串最大长度N: ";
            cin >> N;
            cout << "长度≤" << N << "的所有合法字符串：" << endl;
            generateAllStrings(N);
        }
        else if (choice == 2)
        {
            // 手动输入字符串判定
            string s;
            cout << "请输入待判定的字符串: ";
            cin >> s;
            simulateProcess(s);
        }
        else if (choice == 3)
        {
            // 随机生成字符串判定
            int len;
            cout << "请输入随机字符串长度: ";
            cin >> len;
            string s = randomString(len);
            cout << "随机生成的字符串: " << s << endl;
            simulateProcess(s);
        }
        else if (choice == 0)
        {
            cout << "程序退出！" << endl;
            break;
        }
        else
        {
            cout << "输入错误，请重新选择！" << endl;
        }
        cout << "----------------------------------------" << endl;
    }

    return 0;
}

// 从文本文件读取DFA五元组
bool readDFAFromFile(const string &filename)
{
    ifstream fin(filename);
    if (!fin.is_open())
        return false;

    // 1. 读取字符集
    int sigmaSize;
    fin >> sigmaSize;
    for (int i = 0; i < sigmaSize; ++i)
    {
        char c;
        fin >> c;
        dfa.sigma.insert(c);
    }

    // 2. 读取状态集
    int stateSize;
    fin >> stateSize;
    for (int i = 0; i < stateSize; ++i)
    {
        int s;
        fin >> s;
        dfa.states.insert(s);
    }

    // 3. 读取开始状态
    fin >> dfa.start;

    // 4. 读取接受状态集
    int acceptSize;
    fin >> acceptSize;
    for (int i = 0; i < acceptSize; ++i)
    {
        int a;
        fin >> a;
        dfa.accept.insert(a);
    }

    // 5. 读取状态转换表 (当前状态 字符 下一状态)
    int transSize;
    fin >> transSize;
    for (int i = 0; i < transSize; ++i)
    {
        int from, to;
        char c;
        fin >> from >> c >> to;
        dfa.trans[{from, c}] = to;
    }

    fin.close();
    return true;
}

// 检查DFA合法性
bool checkDFA()
{
    // 1. 开始状态必须唯一（文件只读取一个，天然唯一）
    // 2. 开始状态必须属于状态集
    if (dfa.states.find(dfa.start) == dfa.states.end())
    {
        cout << "错误：开始状态不在状态集中！" << endl;
        return false;
    }

    // 3. 接受状态集不能为空
    if (dfa.accept.empty())
    {
        cout << "错误：接受状态集为空！" << endl;
        return false;
    }

    // 4. 所有接受状态必须属于状态集
    for (int a : dfa.accept)
    {
        if (dfa.states.find(a) == dfa.states.end())
        {
            cout << "错误：接受状态" << a << "不在状态集中！" << endl;
            return false;
        }
    }

    return true;
}

// 生成所有长度<=maxLen的合法字符串
void generateAllStrings(int maxLen)
{
    dfsGenerate(dfa.start, "", maxLen);
}

// 深度优先搜索生成字符串（核心递归函数）
void dfsGenerate(int curState, string curStr, int maxLen)
{
    // 如果当前字符串长度超过最大值，终止
    if (curStr.size() > maxLen)
        return;

    // 如果当前状态是接受状态，输出字符串
    if (dfa.accept.count(curState))
    {
        // 空字符串特殊处理
        if (curStr.empty())
            cout << "ε (空串)" << endl;
        else
            cout << curStr << endl;
    }

    // 遍历所有字符，递归生成
    for (char c : dfa.sigma)
    {
        // 查找转换表
        auto it = dfa.trans.find({curState, c});
        if (it != dfa.trans.end())
        {
            dfsGenerate(it->second, curStr + c, maxLen);
        }
    }
}

// 判定字符串是否合法
bool judgeString(const string &str)
{
    int cur = dfa.start;
    for (char c : str)
    {
        // 字符不在字符集中，直接不合法
        if (dfa.sigma.find(c) == dfa.sigma.end())
            return false;
        // 无对应转换规则，不合法
        auto it = dfa.trans.find({cur, c});
        if (it == dfa.trans.end())
            return false;
        cur = it->second;
    }
    // 最终状态是否为接受状态
    return dfa.accept.count(cur);
}

// 模拟DFA识别字符串的完整过程
void simulateProcess(const string &str)
{
    cout << "=== DFA识别过程模拟 ===" << endl;
    int cur = dfa.start;
    cout << "初始状态: " << cur << endl;

    for (int i = 0; i < str.size(); ++i)
    {
        char c = str[i];
        cout << "第" << i + 1 << "步，输入字符: " << c;

        // 字符非法
        if (dfa.sigma.find(c) == dfa.sigma.end())
        {
            cout << " → 字符不在字符集中，识别失败！" << endl;
            return;
        }

        // 无转换规则
        auto it = dfa.trans.find({cur, c});
        if (it == dfa.trans.end())
        {
            cout << " → 无状态转换规则，识别失败！" << endl;
            return;
        }

        cur = it->second;
        cout << " → 转换到状态: " << cur << endl;
    }

    // 最终判定
    if (dfa.accept.count(cur))
    {
        cout << "最终状态为接受状态 → 该字符串是【合法字符串】" << endl;
    }
    else
    {
        cout << "最终状态不是接受状态 → 该字符串是【非法字符串】" << endl;
    }
}

// 随机生成指定长度的字符串（从字符集中选取）
string randomString(int len)
{
    string res;
    vector<char> chars(dfa.sigma.begin(), dfa.sigma.end());
    for (int i = 0; i < len; ++i)
    {
        int idx = rand() % chars.size();
        res += chars[idx];
    }
    return res;
}

// 显示功能菜单
void showMenu()
{
    cout << "\n===== DFA模拟系统 =====" << endl;
    cout << "1. 输出所有长度≤N的合法字符串" << endl;
    cout << "2. 手动输入字符串判定合法性" << endl;
    cout << "3. 随机生成字符串判定合法性" << endl;
    cout << "0. 退出程序" << endl;
    cout << "请输入你的选择: ";
}