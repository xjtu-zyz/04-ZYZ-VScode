#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Production
{
    string lhs;
    vector<string> rhs;
};

struct Item
{
    int production = 0;
    int dot = 0;

    Item() {}
    Item(int productionIndex, int dotPosition)
        : production(productionIndex), dot(dotPosition) {}

    bool operator<(const Item &other) const
    {
        if (production != other.production)
        {
            return production < other.production;
        }
        return dot < other.dot;
    }
};

struct Grammar
{
    string startSymbol;
    string augmentedStart;
    vector<Production> productions;
    map<string, vector<int> > byLhs;
    set<string> nonterminals;
    set<string> terminals;
};

static string trim(const string &text)
{
    size_t left = text.find_first_not_of(" \t\r\n");
    if (left == string::npos)
    {
        return "";
    }
    size_t right = text.find_last_not_of(" \t\r\n");
    return text.substr(left, right - left + 1);
}

static vector<string> splitWords(const string &text)
{
    vector<string> result;
    stringstream ss(text);
    string word;
    while (ss >> word)
    {
        result.push_back(word);
    }
    return result;
}

static vector<string> splitAlternatives(const string &text)
{
    vector<string> result;
    string current;
    for (char ch : text)
    {
        if (ch == '|')
        {
            result.push_back(trim(current));
            current.clear();
        }
        else
        {
            current += ch;
        }
    }
    result.push_back(trim(current));
    return result;
}

static string stripBom(string text)
{
    if (text.size() >= 3 &&
        static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF)
    {
        text.erase(0, 3);
    }
    return text;
}

static string readAll(const string &filename)
{
    ifstream fin(filename.c_str(), ios::in | ios::binary);
    if (!fin.is_open())
    {
        return "";
    }
    stringstream buffer;
    buffer << fin.rdbuf();
    return stripBom(buffer.str());
}

static bool isNonterminalToken(const string &symbol)
{
    return !symbol.empty() && isupper(static_cast<unsigned char>(symbol[0]));
}

static string joinSymbols(const vector<string> &symbols, const string &separator = " ")
{
    string result;
    for (size_t i = 0; i < symbols.size(); ++i)
    {
        if (i)
        {
            result += separator;
        }
        result += symbols[i];
    }
    return result;
}

static string productionText(const Production &production)
{
    return production.lhs + " -> " + (production.rhs.empty() ? "ε" : joinSymbols(production.rhs));
}

static string itemText(const Grammar &grammar, const Item &item)
{
    const Production &production = grammar.productions[item.production];
    vector<string> symbols = production.rhs;
    symbols.insert(symbols.begin() + item.dot, "·");
    return production.lhs + " -> " + (symbols.empty() ? "·" : joinSymbols(symbols));
}

static void rebuildIndexes(Grammar &grammar)
{
    grammar.byLhs.clear();
    grammar.nonterminals.clear();
    grammar.terminals.clear();

    for (const Production &production : grammar.productions)
    {
        grammar.nonterminals.insert(production.lhs);
    }

    for (int i = 0; i < (int)grammar.productions.size(); ++i)
    {
        const Production &production = grammar.productions[i];
        grammar.byLhs[production.lhs].push_back(i);
        for (const string &symbol : production.rhs)
        {
            if (grammar.nonterminals.count(symbol))
            {
                continue;
            }
            grammar.terminals.insert(symbol);
        }
    }
}

static Grammar parseGrammarFile(const string &filename)
{
    string text = readAll(filename);
    if (text.empty())
    {
        throw runtime_error("无法读取文法文件：" + filename);
    }

    Grammar grammar;
    vector<Production> originalProductions;
    string line;
    stringstream input(text);
    while (getline(input, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        size_t arrow = line.find("->");
        if (arrow == string::npos)
        {
            arrow = line.find("=>");
        }
        if (arrow == string::npos)
        {
            throw runtime_error("文法行缺少 -> ：" + line);
        }

        string lhs = trim(line.substr(0, arrow));
        string rhsText = trim(line.substr(arrow + 2));
        if (lhs.empty())
        {
            throw runtime_error("产生式左部不能为空：" + line);
        }

        if (grammar.startSymbol.empty())
        {
            grammar.startSymbol = lhs;
        }

        for (const string &alternative : splitAlternatives(rhsText))
        {
            vector<string> rhs;
            if (alternative != "ε" && alternative != "epsilon" && alternative != "@")
            {
                rhs = splitWords(alternative);
            }
            originalProductions.push_back({lhs, rhs});
        }
    }

    if (originalProductions.empty())
    {
        throw runtime_error("文法文件中没有有效产生式。");
    }

    grammar.augmentedStart = grammar.startSymbol + "'";
    while (isNonterminalToken(grammar.augmentedStart))
    {
        bool exists = false;
        for (const Production &production : originalProductions)
        {
            if (production.lhs == grammar.augmentedStart)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            break;
        }
        grammar.augmentedStart += "'";
    }

    grammar.productions.push_back({grammar.augmentedStart, vector<string>{grammar.startSymbol}});
    grammar.productions.insert(grammar.productions.end(), originalProductions.begin(), originalProductions.end());
    rebuildIndexes(grammar);
    return grammar;
}

static set<Item> closure(const Grammar &grammar, const set<Item> &items)
{
    set<Item> result = items;
    bool changed = true;
    while (changed)
    {
        changed = false;
        vector<Item> snapshot(result.begin(), result.end());
        for (const Item &item : snapshot)
        {
            const Production &production = grammar.productions[item.production];
            if (item.dot >= (int)production.rhs.size())
            {
                continue;
            }

            string next = production.rhs[item.dot];
            if (!grammar.nonterminals.count(next))
            {
                continue;
            }

            for (int productionIndex : grammar.byLhs.at(next))
            {
                Item newItem{productionIndex, 0};
                if (!result.count(newItem))
                {
                    result.insert(newItem);
                    changed = true;
                }
            }
        }
    }
    return result;
}

static set<Item> goTo(const Grammar &grammar, const set<Item> &items, const string &symbol)
{
    set<Item> moved;
    for (const Item &item : items)
    {
        const Production &production = grammar.productions[item.production];
        if (item.dot < (int)production.rhs.size() && production.rhs[item.dot] == symbol)
        {
            moved.insert({item.production, item.dot + 1});
        }
    }
    if (moved.empty())
    {
        return moved;
    }
    return closure(grammar, moved);
}

static string itemSetKey(const set<Item> &items)
{
    string key;
    for (const Item &item : items)
    {
        key += to_string(item.production) + "." + to_string(item.dot) + ";";
    }
    return key;
}

static vector<string> grammarSymbols(const Grammar &grammar)
{
    vector<string> symbols;
    for (const string &symbol : grammar.terminals)
    {
        symbols.push_back(symbol);
    }
    for (const string &symbol : grammar.nonterminals)
    {
        if (symbol != grammar.augmentedStart)
        {
            symbols.push_back(symbol);
        }
    }
    return symbols;
}

static void buildCanonicalCollection(const Grammar &grammar,
                                     vector<set<Item> > &states,
                                     map<pair<int, string>, int> &transitions)
{
    set<Item> start;
    start.insert({0, 0});
    start = closure(grammar, start);

    map<string, int> ids;
    states.push_back(start);
    ids[itemSetKey(start)] = 0;

    queue<int> pending;
    pending.push(0);
    vector<string> symbols = grammarSymbols(grammar);

    while (!pending.empty())
    {
        int current = pending.front();
        pending.pop();

        for (const string &symbol : symbols)
        {
            set<Item> next = goTo(grammar, states[current], symbol);
            if (next.empty())
            {
                continue;
            }

            string key = itemSetKey(next);
            if (!ids.count(key))
            {
                int nextId = (int)states.size();
                ids[key] = nextId;
                states.push_back(next);
                pending.push(nextId);
            }
            transitions[{current, symbol}] = ids[key];
        }
    }
}

static vector<string> detectConflicts(const Grammar &grammar,
                                      const vector<set<Item> > &states)
{
    vector<string> conflicts;
    for (int i = 0; i < (int)states.size(); ++i)
    {
        int reduceCount = 0;
        bool hasShift = false;
        vector<string> reduceItems;

        for (const Item &item : states[i])
        {
            const Production &production = grammar.productions[item.production];
            bool completed = item.dot == (int)production.rhs.size();
            if (completed && item.production != 0)
            {
                ++reduceCount;
                reduceItems.push_back(itemText(grammar, item));
            }
            else if (!completed)
            {
                string next = production.rhs[item.dot];
                if (grammar.terminals.count(next))
                {
                    hasShift = true;
                }
            }
        }

        if (reduceCount > 1)
        {
            conflicts.push_back("I" + to_string(i) + " 存在归约-归约冲突：" + joinSymbols(reduceItems, "；"));
        }
        if (reduceCount > 0 && hasShift)
        {
            conflicts.push_back("I" + to_string(i) + " 存在移进-归约冲突：" + joinSymbols(reduceItems, "；"));
        }
    }
    return conflicts;
}

static string escapeDotLabel(const string &text)
{
    string result;
    for (char ch : text)
    {
        if (ch == '"' || ch == '\\')
        {
            result += '\\';
        }
        if (ch == '\n')
        {
            result += "\\n";
        }
        else
        {
            result += ch;
        }
    }
    return result;
}

static void writeDotFile(const Grammar &grammar,
                         const vector<set<Item> > &states,
                         const map<pair<int, string>, int> &transitions,
                         const string &filename)
{
    ofstream out(filename.c_str());
    out << "digraph LR0Items {\n";
    out << "  rankdir=LR;\n";
    out << "  graph [label=\"LR(0)项目集规范族\", labelloc=t, fontsize=24, fontname=\"Microsoft YaHei\"];\n";
    out << "  node [shape=box, fontname=\"Consolas\", fontsize=14];\n";
    out << "  edge [fontname=\"Consolas\", fontsize=15];\n";

    for (int i = 0; i < (int)states.size(); ++i)
    {
        string label = "I" + to_string(i) + "\\n";
        for (const Item &item : states[i])
        {
            label += itemText(grammar, item) + "\\n";
        }
        out << "  I" << i << " [label=\"" << escapeDotLabel(label) << "\"];\n";
    }

    for (const auto &transition : transitions)
    {
        out << "  I" << transition.first.first << " -> I" << transition.second
            << " [label=\"" << escapeDotLabel(transition.first.second) << "\"];\n";
    }
    out << "}\n";
}

static vector<string> tokenizeExpression(const string &text)
{
    vector<string> tokens;
    for (size_t i = 0; i < text.size();)
    {
        char ch = text[i];
        if (isspace(static_cast<unsigned char>(ch)))
        {
            ++i;
            continue;
        }
        if (isalpha(static_cast<unsigned char>(ch)))
        {
            while (i < text.size() && (isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_'))
            {
                ++i;
            }
            tokens.push_back("a");
            continue;
        }
        if (isdigit(static_cast<unsigned char>(ch)))
        {
            while (i < text.size() && isdigit(static_cast<unsigned char>(text[i])))
            {
                ++i;
            }
            tokens.push_back("a");
            continue;
        }
        tokens.push_back(string(1, ch));
        ++i;
    }
    return tokens;
}

static string mapScannerToken(const string &type, const string &lexeme)
{
    if (type == "ID" || type == "NUM" || type == "FLO")
    {
        return "a";
    }
    if (type == "ADD")
    {
        return "+";
    }
    if (type == "MUL")
    {
        return "*";
    }
    if (type == "LPA")
    {
        return "(";
    }
    if (type == "RPA")
    {
        return ")";
    }
    if (type == "SCO")
    {
        return ";";
    }
    if (type == "ASG")
    {
        return "=";
    }
    return lexeme;
}

static vector<string> tokenizeScannerOutput(const string &text)
{
    vector<string> tokens;
    string line;
    stringstream input(text);
    while (getline(input, line))
    {
        line = trim(line);
        if (line.empty() || line[0] != '(')
        {
            continue;
        }

        size_t comma = line.find(',');
        size_t right = line.find(')', comma == string::npos ? 0 : comma);
        if (comma == string::npos || right == string::npos)
        {
            continue;
        }

        string type = trim(line.substr(1, comma - 1));
        string lexeme = trim(line.substr(comma + 1, right - comma - 1));
        tokens.push_back(mapScannerToken(type, lexeme));
    }
    return tokens;
}

static vector<string> tokenizeInputForGrammar(const string &text)
{
    if (text.find('(') != string::npos && text.find(',') != string::npos)
    {
        vector<string> fromScanner = tokenizeScannerOutput(text);
        if (!fromScanner.empty())
        {
            return fromScanner;
        }
    }
    return tokenizeExpression(text);
}

static void printGrammar(const Grammar &grammar)
{
    cout << "===== 增广文法 =====\n";
    for (int i = 0; i < (int)grammar.productions.size(); ++i)
    {
        cout << setw(2) << i << ". " << productionText(grammar.productions[i]) << '\n';
    }
}

static void printStates(const Grammar &grammar,
                        const vector<set<Item> > &states,
                        const map<pair<int, string>, int> &transitions)
{
    cout << "\n===== LR(0)项目集规范族 =====\n";
    for (int i = 0; i < (int)states.size(); ++i)
    {
        cout << "I" << i << ":\n";
        for (const Item &item : states[i])
        {
            cout << "  " << itemText(grammar, item) << '\n';
        }
        cout << '\n';
    }

    cout << "===== 每个状态的闭包 Closure =====\n";
    cout << "说明：规范族中的每个状态 Ii 本身就是对核心项目集反复执行 closure 后得到的闭包。\n";
    for (int i = 0; i < (int)states.size(); ++i)
    {
        set<Item> closed = closure(grammar, states[i]);
        cout << "Closure(I" << i << "):\n";
        for (const Item &item : closed)
        {
            cout << "  " << itemText(grammar, item) << '\n';
        }
        cout << '\n';
    }

    cout << "===== GOTO状态转移 =====\n";
    for (const auto &transition : transitions)
    {
        cout << "  GOTO(I" << transition.first.first << ", '"
             << transition.first.second << "') = I" << transition.second << '\n';
    }
}

static void printConflictResult(const vector<string> &conflicts)
{
    cout << "\n===== LR(0)冲突检查 =====\n";
    cout << "判断思路：若某个项目集中同时存在移进项目和归约项目，则为移进-归约冲突；"
         << "若同一项目集中存在两个及以上归约项目，则为归约-归约冲突。"
         << "没有上述冲突时，可判定为LR(0)文法。\n";
    if (conflicts.empty())
    {
        cout << "未发现移进-归约冲突或归约-归约冲突，该文法可视为LR(0)文法。\n";
        return;
    }

    cout << "发现冲突，该文法不是LR(0)文法：\n";
    for (const string &conflict : conflicts)
    {
        cout << "  " << conflict << '\n';
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    string grammarFile = argc >= 2 ? argv[1] : "grammar.txt";
    string sourceFile = argc >= 3 ? argv[2] : "";
    string outputFile = argc >= 4 ? argv[3] : "";

    ofstream output;
    streambuf *oldCout = nullptr;
    if (!outputFile.empty())
    {
        output.open(outputFile.c_str(), ios::out | ios::binary);
        if (!output.is_open())
        {
            cerr << "错误：无法创建输出文件：" << outputFile << '\n';
            return 1;
        }

        // Write UTF-8 BOM so Windows Notepad/Word can identify the file encoding reliably.
        output << "\xEF\xBB\xBF";
        oldCout = cout.rdbuf(output.rdbuf());
    }

    try
    {
        Grammar grammar = parseGrammarFile(grammarFile);
        vector<set<Item> > states;
        map<pair<int, string>, int> transitions;
        buildCanonicalCollection(grammar, states, transitions);
        vector<string> conflicts = detectConflicts(grammar, states);

        cout << "===== 实验三：LR(0)语法分析 =====\n";
        cout << "文法文件：" << grammarFile << "\n";
        printGrammar(grammar);
        printStates(grammar, states, transitions);
        printConflictResult(conflicts);

        writeDotFile(grammar, states, transitions, "lr0_items.dot");
        cout << "\n状态图Graphviz文件已生成：lr0_items.dot\n";
        cout << "可使用命令生成图片：dot -Tpng lr0_items.dot -o lr0_items.png\n";

        if (!sourceFile.empty())
        {
            string source = readAll(sourceFile);
            cout << "\n===== 源程序符号串简单归一化 =====\n";
            cout << "源文件：" << sourceFile << '\n';
            vector<string> tokens = tokenizeInputForGrammar(source);
            cout << "归一化终结符序列：";
            for (const string &token : tokens)
            {
                cout << token << ' ';
            }
            cout << "$\n";
        }

        if (oldCout)
        {
            cout.rdbuf(oldCout);
            cout << "运行结果已按 UTF-8 编码保存到：" << outputFile << '\n';
        }
    }
    catch (const exception &ex)
    {
        if (oldCout)
        {
            cout.rdbuf(oldCout);
        }
        cerr << "错误：" << ex.what() << '\n';
        return 1;
    }

    return 0;
}
