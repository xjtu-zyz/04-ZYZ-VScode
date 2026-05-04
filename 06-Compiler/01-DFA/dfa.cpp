#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// DFA结构体：存储五元组
struct DFA
{
    set<char> sigma;                 // 字符集
    set<int> states;                 // 状态集
    int start = -1;                  // 开始状态
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

// 去除字符串首尾空白字符
string trim(const string &s)
{
    size_t l = s.find_first_not_of(" \t\r\n");
    if (l == string::npos)
        return "";
    size_t r = s.find_last_not_of(" \t\r\n");
    return s.substr(l, r - l + 1);
}

// 读取一行，并跳过空行和注释行
bool getValidLine(ifstream &fin, string &line)
{
    while (getline(fin, line))
    {
        line = trim(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;
        return true;
    }
    return false;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    srand((unsigned)time(0));

    string filename;
    cout << "请输入DFA文件名(如dfa.txt): ";
    cin >> filename;

    if (!readDFAFromFile(filename))
    {
        cout << "文件读取失败！程序退出" << endl;
        return 1;
    }
    cout << "DFA读取成功！" << endl;

    if (!checkDFA())
    {
        cout << "DFA不合法！程序退出" << endl;
        return 1;
    }
    cout << "DFA合法性检查通过！" << endl;

    while (true)
    {
        showMenu();
        int choice;
        cin >> choice;

        if (choice == 1)
        {
            int N;
            cout << "请输入字符串最大长度N: ";
            cin >> N;
            if (N < 0)
            {
                cout << "N不能为负数！" << endl;
                continue;
            }
            cout << "长度≤" << N << "的所有合法字符串：" << endl;
            generateAllStrings(N);
        }
        else if (choice == 2)
        {
            string s;
            cout << "请输入待判定的字符串，空串请输入EPS: ";
            cin >> s;
            if (s == "EPS" || s == "eps")
                s = "";
            simulateProcess(s);
        }
        else if (choice == 3)
        {
            int len;
            cout << "请输入随机字符串长度: ";
            cin >> len;
            if (len < 0)
            {
                cout << "长度不能为负数！" << endl;
                continue;
            }
            string s = randomString(len);
            cout << "随机生成的字符串: " << (s.empty() ? "EPS" : s) << endl;
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

/*
简洁输入文件格式：
第1行：字符集，例如 a b
第2行：状态集，例如 0 1
第3行：开始状态，例如 0
第4行：接受状态集，例如 0
第5行及以后：状态转换表，每行格式为：当前状态 输入字符 下一状态

示例：
a b
0 1
0
0
0 a 1
0 b 0
1 a 0
1 b 1
*/
bool readDFAFromFile(const string &filename)
{
    ifstream fin(filename);
    if (!fin.is_open())
        return false;

    dfa = DFA(); // 每次读取前清空旧数据

    string line;

    // 1. 读取字符集：一行直接写所有字符，用空格分隔
    if (!getValidLine(fin, line))
    {
        cout << "错误：缺少字符集行！" << endl;
        return false;
    }
    stringstream ssSigma(line);
    char c;
    while (ssSigma >> c)
        dfa.sigma.insert(c);

    // 2. 读取状态集：一行直接写所有状态编号，用空格分隔
    if (!getValidLine(fin, line))
    {
        cout << "错误：缺少状态集行！" << endl;
        return false;
    }
    stringstream ssStates(line);
    int state;
    while (ssStates >> state)
        dfa.states.insert(state);

    // 3. 读取开始状态：一行只写一个状态编号
    if (!getValidLine(fin, line))
    {
        cout << "错误：缺少开始状态行！" << endl;
        return false;
    }
    stringstream ssStart(line);
    if (!(ssStart >> dfa.start))
    {
        cout << "错误：开始状态读取失败！" << endl;
        return false;
    }
    int extraStart;
    if (ssStart >> extraStart)
    {
        cout << "错误：开始状态必须唯一，一行只能写一个状态！" << endl;
        return false;
    }

    // 4. 读取接受状态集：一行写所有接受状态，用空格分隔
    if (!getValidLine(fin, line))
    {
        cout << "错误：缺少接受状态集行！" << endl;
        return false;
    }
    stringstream ssAccept(line);
    int acc;
    while (ssAccept >> acc)
        dfa.accept.insert(acc);

    // 5. 读取状态转换表：剩余每一行都是 from char to
    while (getValidLine(fin, line))
    {
        stringstream ssTrans(line);
        int from, to;
        char ch;
        if (!(ssTrans >> from >> ch >> to))
        {
            cout << "错误：转换表格式错误，应为：当前状态 输入字符 下一状态。错误行：" << line << endl;
            return false;
        }

        string extra;
        if (ssTrans >> extra)
        {
            cout << "错误：转换表每行只能有三项。错误行：" << line << endl;
            return false;
        }

        pair<int, char> key = {from, ch};
        if (dfa.trans.count(key))
        {
            cout << "错误：存在重复转换：(" << from << ", " << ch << ")" << endl;
            return false;
        }
        dfa.trans[key] = to;
    }

    fin.close();
    return true;
}

bool checkDFA()
{
    if (dfa.sigma.empty())
    {
        cout << "错误：字符集为空！" << endl;
        return false;
    }

    if (dfa.states.empty())
    {
        cout << "错误：状态集为空！" << endl;
        return false;
    }

    if (dfa.states.find(dfa.start) == dfa.states.end())
    {
        cout << "错误：开始状态 " << dfa.start << " 不在状态集中！" << endl;
        return false;
    }

    if (dfa.accept.empty())
    {
        cout << "错误：接受状态集为空！" << endl;
        return false;
    }

    for (int a : dfa.accept)
    {
        if (dfa.states.find(a) == dfa.states.end())
        {
            cout << "错误：接受状态 " << a << " 不在状态集中！" << endl;
            return false;
        }
    }

    // 检查转换表中出现的状态和字符是否合法
    for (auto &item : dfa.trans)
    {
        int from = item.first.first;
        char ch = item.first.second;
        int to = item.second;

        if (!dfa.states.count(from))
        {
            cout << "错误：转换表中的当前状态 " << from << " 不在状态集中！" << endl;
            return false;
        }
        if (!dfa.sigma.count(ch))
        {
            cout << "错误：转换表中的输入字符 " << ch << " 不在字符集中！" << endl;
            return false;
        }
        if (!dfa.states.count(to))
        {
            cout << "错误：转换表中的下一状态 " << to << " 不在状态集中！" << endl;
            return false;
        }
    }

    // DFA要求：每个状态在每个字符下都必须有唯一的下一状态
    for (int s : dfa.states)
    {
        for (char ch : dfa.sigma)
        {
            if (!dfa.trans.count({s, ch}))
            {
                cout << "错误：缺少转换 δ(" << s << ", " << ch << ")！" << endl;
                return false;
            }
        }
    }

    return true;
}

void generateAllStrings(int maxLen)
{
    dfsGenerate(dfa.start, "", maxLen);
}

void dfsGenerate(int curState, string curStr, int maxLen)
{
    if ((int)curStr.size() > maxLen)
        return;

    if (dfa.accept.count(curState))
    {
        if (curStr.empty())
            cout << "EPS (空串)" << endl;
        else
            cout << curStr << endl;
    }

    if ((int)curStr.size() == maxLen)
        return;

    for (char ch : dfa.sigma)
    {
        auto it = dfa.trans.find({curState, ch});
        if (it != dfa.trans.end())
            dfsGenerate(it->second, curStr + ch, maxLen);
    }
}

bool judgeString(const string &str)
{
    int cur = dfa.start;
    for (char ch : str)
    {
        if (!dfa.sigma.count(ch))
            return false;

        auto it = dfa.trans.find({cur, ch});
        if (it == dfa.trans.end())
            return false;

        cur = it->second;
    }
    return dfa.accept.count(cur);
}

void simulateProcess(const string &str)
{
    cout << "=== DFA识别过程模拟 ===" << endl;
    int cur = dfa.start;
    cout << "初始状态: " << cur << endl;

    if (str.empty())
        cout << "输入为空串 EPS，不进行字符读取。" << endl;

    for (int i = 0; i < (int)str.size(); ++i)
    {
        char ch = str[i];
        cout << "第" << i + 1 << "步，输入字符: " << ch;

        if (!dfa.sigma.count(ch))
        {
            cout << " → 字符不在字符集中，识别失败！" << endl;
            return;
        }

        auto it = dfa.trans.find({cur, ch});
        if (it == dfa.trans.end())
        {
            cout << " → 无状态转换规则，识别失败！" << endl;
            return;
        }

        cout << "，状态变化: " << cur << " -> " << it->second << endl;
        cur = it->second;
    }

    cout << "最终状态: " << cur << endl;
    if (dfa.accept.count(cur))
        cout << "最终状态为接受状态 → 该字符串是【合法字符串】" << endl;
    else
        cout << "最终状态不是接受状态 → 该字符串是【非法字符串】" << endl;
}

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

void showMenu()
{
    cout << "\n===== DFA模拟系统 =====" << endl;
    cout << "1. 输出所有长度≤N的合法字符串" << endl;
    cout << "2. 手动输入字符串判定合法性" << endl;
    cout << "3. 随机生成字符串判定合法性" << endl;
    cout << "0. 退出程序" << endl;
    cout << "请输入你的选择: ";
}
