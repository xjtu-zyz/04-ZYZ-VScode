#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

enum class LabTokenType
{
    SCO,
    ID,
    INT,
    FLO,
    AAS,
    ADD,
    SUB,
    DOT,
    ERR
};

struct Token
{
    LabTokenType type;
    string lexeme;
    string path;
};

static bool isLetter(char ch)
{
    return isalpha(static_cast<unsigned char>(ch)) != 0;
}

static bool isDigit(char ch)
{
    return isdigit(static_cast<unsigned char>(ch)) != 0;
}

static string tokenName(LabTokenType type)
{
    switch (type)
    {
    case LabTokenType::SCO:
        return "SCO";
    case LabTokenType::ID:
        return "ID";
    case LabTokenType::INT:
        return "INT";
    case LabTokenType::FLO:
        return "FLO";
    case LabTokenType::AAS:
        return "AAS";
    case LabTokenType::ADD:
        return "ADD";
    case LabTokenType::SUB:
        return "SUB";
    case LabTokenType::DOT:
        return "DOT";
    default:
        return "ERROR";
    }
}

static string tokenDescription(LabTokenType type)
{
    switch (type)
    {
    case LabTokenType::SCO:
        return "分号";
    case LabTokenType::ID:
        return "标识符";
    case LabTokenType::INT:
        return "整数";
    case LabTokenType::FLO:
        return "浮点数";
    case LabTokenType::AAS:
        return "赋值号";
    case LabTokenType::ADD:
        return "加号";
    case LabTokenType::SUB:
        return "减号";
    case LabTokenType::DOT:
        return "点号";
    default:
        return "无法识别的字符";
    }
}

static bool canStartSignedNumber(const string &text, size_t pos)
{
    if (pos + 1 >= text.size() || !isDigit(text[pos + 1]))
    {
        return false;
    }

    if (pos == 0)
    {
        return true;
    }

    char prev = text[pos - 1];
    return isspace(static_cast<unsigned char>(prev)) || prev == '=' || prev == '+' || prev == '-' || prev == ';';
}

static Token scanNumberDFA(const string &text, size_t &pos)
{
    size_t start = pos;
    string path = "0";

    if (text[pos] == '-')
    {
        path += "->A";
        ++pos;
    }

    bool hasDigitBeforeDot = false;
    while (pos < text.size() && isDigit(text[pos]))
    {
        hasDigitBeforeDot = true;
        path += "->B";
        ++pos;
    }

    bool isFloat = false;
    if (pos < text.size() && text[pos] == '.')
    {
        isFloat = true;
        path += "->C";
        ++pos;
        while (pos < text.size() && isDigit(text[pos]))
        {
            path += "->D";
            ++pos;
        }
    }

    if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E'))
    {
        isFloat = true;
        size_t expStart = pos;
        string expPath = path + "->E";
        ++pos;

        if (pos < text.size() && (text[pos] == '+' || text[pos] == '-'))
        {
            expPath += "->F";
            ++pos;
        }

        bool hasExpDigit = false;
        while (pos < text.size() && isDigit(text[pos]))
        {
            hasExpDigit = true;
            expPath += "->G";
            ++pos;
        }

        if (hasExpDigit)
        {
            path = expPath;
        }
        else
        {
            pos = expStart;
        }
    }

    Token token;
    token.lexeme = text.substr(start, pos - start);
    token.path = path;
    token.type = (isFloat || !hasDigitBeforeDot) ? LabTokenType::FLO : LabTokenType::INT;
    return token;
}

static vector<Token> tokenize(const string &text)
{
    vector<Token> tokens;
    size_t pos = 0;

    while (pos < text.size())
    {
        char ch = text[pos];
        if (isspace(static_cast<unsigned char>(ch)))
        {
            ++pos;
            continue;
        }

        if (isLetter(ch))
        {
            size_t start = pos++;
            string path = "0->8";
            while (pos < text.size() && (isLetter(text[pos]) || isDigit(text[pos])))
            {
                path += "->8";
                ++pos;
            }
            tokens.push_back({LabTokenType::ID, text.substr(start, pos - start), path});
            continue;
        }

        if (isDigit(ch) || (ch == '-' && canStartSignedNumber(text, pos)))
        {
            tokens.push_back(scanNumberDFA(text, pos));
            continue;
        }

        if (ch == '.' && pos + 1 < text.size() && isDigit(text[pos + 1]))
        {
            size_t start = pos++;
            string path = "0->C";
            while (pos < text.size() && isDigit(text[pos]))
            {
                path += "->D";
                ++pos;
            }
            tokens.push_back({LabTokenType::FLO, text.substr(start, pos - start), path});
            continue;
        }

        Token token;
        token.lexeme = string(1, ch);
        switch (ch)
        {
        case ';':
            token = {LabTokenType::SCO, token.lexeme, "0->2"};
            break;
        case '=':
            token = {LabTokenType::AAS, token.lexeme, "0->5"};
            break;
        case '+':
            token = {LabTokenType::ADD, token.lexeme, "0->4->6"};
            break;
        case '-':
            token = {LabTokenType::SUB, token.lexeme, "0->A"};
            break;
        case '.':
            token = {LabTokenType::DOT, token.lexeme, "0->C"};
            break;
        default:
            token = {LabTokenType::ERR, token.lexeme, "0->ERROR"};
            break;
        }
        tokens.push_back(token);
        ++pos;
    }

    return tokens;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    cout << "===== 实验二：复杂DFA词法单位识别 =====\n";
    cout << "可识别Token：\n";
    cout << "  SCO  分号 ;\n";
    cout << "  ID   标识符：字母开头，后接字母或数字\n";
    cout << "  INT  整数：数字序列，可识别负整数\n";
    cout << "  FLO  浮点数：支持 123.45、66.、.66、1e-3、9.8E7\n";
    cout << "  AAS  赋值号 =\n";
    cout << "  ADD  加号 +\n";
    cout << "  SUB  减号 -\n";
    cout << "  DOT  点号 .\n";
    cout << "请输入一段源程序，输入结束后按 Ctrl+Z 再按 Enter：\n";
    cout << "示例：a=123.45; b=-123; c=.66; d=1e-3; x+y-z.\n\n";

    string line;
    string source;
    while (getline(cin, line))
    {
        source += line;
        source += '\n';
    }
    if (source.size() >= 3 &&
        static_cast<unsigned char>(source[0]) == 0xEF &&
        static_cast<unsigned char>(source[1]) == 0xBB &&
        static_cast<unsigned char>(source[2]) == 0xBF)
    {
        source.erase(0, 3);
    }

    vector<Token> tokens = tokenize(source);
    cout << "\n===== DFA识别结果 =====\n";
    for (const Token &token : tokens)
    {
        cout << "(" << tokenName(token.type) << ", " << token.lexeme << ")"
             << "  说明：" << tokenDescription(token.type)
             << "  状态路径：" << token.path << '\n';
    }

    return 0;
}
