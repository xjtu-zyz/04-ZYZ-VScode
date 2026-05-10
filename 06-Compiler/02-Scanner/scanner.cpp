#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Token
{
    string type;
    string lexeme;
    int line;
    int column;
};

static const map<string, string> KEYWORDS = {
    {"int", "INT"},
    {"float", "FLOAT"},
    {"void", "VOID"},
    {"if", "IF"},
    {"else", "ELSE"},
    {"while", "WHILE"},
    {"return", "RETURN"},
    {"input", "INPUT"},
    {"print", "PRINT"},
};

static bool isLetter(char ch)
{
    return isalpha(static_cast<unsigned char>(ch)) != 0;
}

static bool isDigit(char ch)
{
    return isdigit(static_cast<unsigned char>(ch)) != 0;
}

static bool isValueEnd(const string &type)
{
    return type == "ID" || type == "NUM" || type == "FLO" || type == "RPA" || type == "RBK";
}

static bool canReadSignedNumber(char sign, const string &source, size_t pos, const vector<Token> &tokens)
{
    if ((sign != '+' && sign != '-') || pos + 1 >= source.size())
    {
        return false;
    }

    if (!isDigit(source[pos + 1]) && source[pos + 1] != '.')
    {
        return false;
    }

    return tokens.empty() || !isValueEnd(tokens.back().type);
}

static string aliasForSampleOutput(const string &type)
{
    if (type == "LPA")
        return "LPAR";
    if (type == "RPA")
        return "RPAR";
    if (type == "SCO")
        return "SEMI";
    if (type == "FLO")
        return "FLOAT";
    return type;
}

static void addToken(vector<Token> &tokens, const string &type, const string &lexeme, int line, int column)
{
    tokens.push_back({type, lexeme, line, column});
}

static Token readIdentifierOrKeyword(const string &source, size_t &pos, int line, int column)
{
    size_t start = pos++;
    while (pos < source.size() && (isLetter(source[pos]) || isDigit(source[pos])))
    {
        ++pos;
    }

    string lexeme = source.substr(start, pos - start);
    auto keyword = KEYWORDS.find(lexeme);
    return {keyword == KEYWORDS.end() ? "ID" : keyword->second, lexeme, line, column};
}

static Token readNumber(const string &source, size_t &pos, int line, int column)
{
    size_t start = pos;

    if (source[pos] == '+' || source[pos] == '-')
    {
        ++pos;
    }

    bool hasDot = false;
    bool hasExp = false;

    while (pos < source.size() && isDigit(source[pos]))
    {
        ++pos;
    }

    if (pos < source.size() && source[pos] == '.')
    {
        hasDot = true;
        ++pos;
        while (pos < source.size() && isDigit(source[pos]))
        {
            ++pos;
        }
    }

    if (pos < source.size() && (source[pos] == 'e' || source[pos] == 'E'))
    {
        size_t expStart = pos;
        ++pos;

        if (pos < source.size() && (source[pos] == '+' || source[pos] == '-'))
        {
            ++pos;
        }

        bool hasExpDigit = false;
        while (pos < source.size() && isDigit(source[pos]))
        {
            hasExpDigit = true;
            ++pos;
        }

        if (hasExpDigit)
        {
            hasExp = true;
        }
        else
        {
            pos = expStart;
        }
    }

    string lexeme = source.substr(start, pos - start);
    return {(hasDot || hasExp) ? "FLO" : "NUM", lexeme, line, column};
}

static vector<Token> scan(const string &source)
{
    vector<Token> tokens;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    while (pos < source.size())
    {
        char ch = source[pos];

        if (ch == '\r')
        {
            ++pos;
            continue;
        }
        if (ch == '\n')
        {
            ++line;
            column = 1;
            ++pos;
            continue;
        }
        if (isspace(static_cast<unsigned char>(ch)))
        {
            ++column;
            ++pos;
            continue;
        }

        if (ch == '/' && pos + 1 < source.size() && source[pos + 1] == '/')
        {
            while (pos < source.size() && source[pos] != '\n')
            {
                ++pos;
                ++column;
            }
            continue;
        }

        int tokenLine = line;
        int tokenColumn = column;
        size_t oldPos = pos;

        if (isLetter(ch))
        {
            tokens.push_back(readIdentifierOrKeyword(source, pos, tokenLine, tokenColumn));
        }
        else if (isDigit(ch) || (ch == '.' && pos + 1 < source.size() && isDigit(source[pos + 1])) ||
                 canReadSignedNumber(ch, source, pos, tokens))
        {
            tokens.push_back(readNumber(source, pos, tokenLine, tokenColumn));
        }
        else
        {
            if (pos + 1 < source.size())
            {
                string two = source.substr(pos, 2);
                if (two == "+=")
                {
                    addToken(tokens, "AAS", two, tokenLine, tokenColumn);
                    pos += 2;
                }
                else if (two == "++")
                {
                    addToken(tokens, "AAA", two, tokenLine, tokenColumn);
                    pos += 2;
                }
                else if (two == "<=" || two == "==" || two == ">=" || two == "!=")
                {
                    addToken(tokens, "ROP", two, tokenLine, tokenColumn);
                    pos += 2;
                }
                else if (two == "&&" || two == "||")
                {
                    addToken(tokens, "BOP", two, tokenLine, tokenColumn);
                    pos += 2;
                }
            }

            if (oldPos == pos)
            {
                switch (ch)
                {
                case '+':
                    addToken(tokens, "ADD", "+", tokenLine, tokenColumn);
                    break;
                case '-':
                    addToken(tokens, "SUB", "-", tokenLine, tokenColumn);
                    break;
                case '*':
                    addToken(tokens, "MUL", "*", tokenLine, tokenColumn);
                    break;
                case '/':
                    addToken(tokens, "DIV", "/", tokenLine, tokenColumn);
                    break;
                case '<':
                case '>':
                    addToken(tokens, "ROP", string(1, ch), tokenLine, tokenColumn);
                    break;
                case '!':
                    addToken(tokens, "BOP", "!", tokenLine, tokenColumn);
                    break;
                case '=':
                    addToken(tokens, "ASG", "=", tokenLine, tokenColumn);
                    break;
                case '(':
                    addToken(tokens, "LPA", "(", tokenLine, tokenColumn);
                    break;
                case ')':
                    addToken(tokens, "RPA", ")", tokenLine, tokenColumn);
                    break;
                case '[':
                    addToken(tokens, "LBK", "[", tokenLine, tokenColumn);
                    break;
                case ']':
                    addToken(tokens, "RBK", "]", tokenLine, tokenColumn);
                    break;
                case '{':
                    addToken(tokens, "LBR", "{", tokenLine, tokenColumn);
                    break;
                case '}':
                    addToken(tokens, "RBR", "}", tokenLine, tokenColumn);
                    break;
                case ',':
                    addToken(tokens, "CMA", ",", tokenLine, tokenColumn);
                    break;
                case ':':
                    addToken(tokens, "COL", ":", tokenLine, tokenColumn);
                    break;
                case ';':
                    addToken(tokens, "SCO", ";", tokenLine, tokenColumn);
                    break;
                default:
                    addToken(tokens, "ERROR", string(1, ch), tokenLine, tokenColumn);
                    break;
                }
                ++pos;
            }
        }

        column += static_cast<int>(pos - oldPos);
    }

    return tokens;
}

static string stripBom(string text);

static string readAll(const string &filename)
{
    ifstream fin(filename.c_str(), ios::in | ios::binary);
    if (!fin.is_open())
    {
        return "";
    }

    stringstream buffer;
    buffer << fin.rdbuf();
    string text = buffer.str();
    return stripBom(text);
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

static void printTypesOnly(const vector<Token> &tokens)
{
    for (const Token &token : tokens)
    {
        cout << aliasForSampleOutput(token.type) << '\n';
    }
}

static void printPairs(const vector<Token> &tokens)
{
    for (const Token &token : tokens)
    {
        cout << "(" << token.type << ", " << token.lexeme << ")\n";
    }
}

static void showMenu()
{
    cout << "===== Scanner 词法分析系统 =====\n";
    cout << "1. 分析符号串类型\n";
    cout << "2. 输出程序语句的token流\n";
    cout << "3. 解析代码文件的token流\n";
    cout << "注：将终端输出到文件需使用：scanner.exe 源文件 [输出文件]\n";
    cout << "请输入你的选择: ";
}

static void runMode1()
{
    int n;
    cout << "请输入符号串个数n: ";
    cin >> n;
    cout << "请输入" << n << "个用空格分隔的符号串: ";
    for (int i = 0; i < n; ++i)
    {
        string word;
        cin >> word;
        vector<Token> tokens = scan(word);
        if (tokens.size() == 1 && tokens[0].lexeme == word)
        {
            cout << aliasForSampleOutput(tokens[0].type) << '\n';
        }
        else
        {
            cout << "ERROR\n";
        }
    }
}

static void runMode2()
{
    string line;
    getline(cin, line);
    cout << "请输入一行源程序语句: ";
    if (line.empty())
    {
        getline(cin, line);
    }
    printTypesOnly(scan(line));
}

static void runMode3()
{
    string filename;
    cout << "请输入源程序文件名: ";
    cin >> filename;
    string source = readAll(filename);
    if (source.empty())
    {
        cerr << "Cannot read file: " << filename << '\n';
        return;
    }
    printPairs(scan(source));
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc >= 2)
    {
        string source = readAll(argv[1]);
        if (source.empty())
        {
            cerr << "Cannot read file: " << argv[1] << '\n';
            return 1;
        }

        if (argc >= 3)
        {
            ofstream fout(argv[2]);
            streambuf *old = cout.rdbuf(fout.rdbuf());
            printPairs(scan(source));
            cout.rdbuf(old);
        }
        else
        {
            printPairs(scan(source));
        }
        return 0;
    }

    showMenu();
    string modeText;
    cin >> modeText;
    modeText = stripBom(modeText);
    int mode = 0;
    stringstream modeStream(modeText);
    modeStream >> mode;
    if (mode == 1)
    {
        runMode1();
    }
    else if (mode == 2)
    {
        runMode2();
    }
    else if (mode == 3)
    {
        runMode3();
    }
    else
    {
        cerr << "Mode must be 1, 2 or 3.\n";
        return 1;
    }

    return 0;
}
