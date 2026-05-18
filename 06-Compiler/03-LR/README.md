# 实验三 LR(0) 语法分析

本目录完成实验三的 LR(0) 项目集规范族构造、状态转移输出、冲突检测，以及实验一扩展 DFA 报错原因分析。

## 文件说明

- `lr0.cpp`：实验三主程序，生成 LR(0) 项目集规范族。
- `grammar.txt`：默认测试文法。
- `test.src`：默认测试源串。
- `lr0.exe`：Windows 下编译得到的可执行文件。
- `lr0_items.dot`：程序运行后生成的 Graphviz 状态图描述文件。
- `lr0_items.svg`：可直接打开或插入实验报告的状态图。
- `lr0_output.txt`：一次运行结果示例。
- `Project3_result.md`：实验结果说明和问题回答。

## 本地 Windows 编译运行

在 `06-Compiler/03-LR` 目录下执行：

```powershell
g++ lr0.cpp -std=c++11 -Wall -Wextra -o lr0.exe
```

默认读取 `grammar.txt`：

```powershell
.\lr0.exe
```

指定文法和源程序：

```powershell
.\lr0.exe grammar.txt test.src
```

保存运行结果：

```powershell
.\lr0.exe grammar.txt test.src > lr0_output.txt
```

更推荐使用程序内置的 UTF-8 输出文件参数，避免 PowerShell 将原生命令输出按 UTF-16 或本地代码页重编码后出现中文乱码：

```powershell
.\lr0.exe grammar.txt test.src lr0_output.txt
```

该方式会直接由程序写出 UTF-8 编码文件，并带 UTF-8 BOM，Windows 记事本、Word 和 VS Code 都能正确识别中文。

## 云服务器 ARM64 操作要求

本次实验环境要求包含鲲鹏 ARM64 云服务器。因此，代码可以先在本地完成，但最终建议在云服务器上重新编译运行一次，并截图放入实验报告。

### 1. 修改 PS1

登录云服务器后，先修改终端提示符：

```bash
export PS1="[\u@\zhouyanzi \W]\$ "
```

该命令只对当前终端会话生效。实验截图时建议保留修改后的提示符，用于体现服务器环境和提交人信息。

### 2. 上传实验三目录

将本地 `06-Compiler/03-LR` 上传到云服务器，例如上传到：

```bash
~/06-Compiler/03-LR
```

进入目录：

```bash
cd ~/06-Compiler/03-LR
```

### 3. 在 ARM64 服务器上编译

```bash
g++ lr0.cpp -std=c++11 -Wall -Wextra -o lr0
```

如果服务器没有安装 `g++`，需要先安装 C++ 编译环境。不同系统命令略有不同，例如 Ubuntu/Debian：

```bash
sudo apt update
sudo apt install g++
```

openEuler/CentOS 类系统可使用：

```bash
sudo yum install gcc-c++
```

### 4. 在 ARM64 服务器上运行

```bash
./lr0 grammar.txt test.src
```

保存输出：

```bash
./lr0 grammar.txt test.src > lr0_output.txt
```

云服务器 Linux 终端一般不会出现 PowerShell 的重编码问题；也可以统一使用程序内置输出文件参数：

```bash
./lr0 grammar.txt test.src lr0_output.txt
```

建议截图内容包括：

- 修改后的 PS1；
- `g++ lr0.cpp -std=c++11 -Wall -Wextra -o lr0` 编译命令；
- `./lr0 grammar.txt test.src` 运行命令；
- 增广文法输出；
- LR(0) 项目集 `I0`、`I1` 等输出；
- GOTO 状态转移；
- LR(0) 冲突检查结果。

### 5. 复现实验一扩展 DFA 报错

`Project3.md` 中要求分析实验一扩展 DFA 扫描 `test.src` 的报错原因。若需要在云服务器上复现，可将实验一或实验二中的 scanner/DFA 程序一并上传，并运行：

```bash
./scanner test.src
```

报错现象类似：

```text
Error: Invalid character 'x' at 1:1
Error: Invalid character '=' at 1:3
Error: Invalid DFA state 3
```

原因总结：

- 扩展 DFA 只接受大写字母作为 ID，不能识别小写 `x`、`y`；
- DFA 没有跳过空格；
- DFA 不支持赋值号 `=`；
- 状态 3 作为 OPERATOR 接受态时没有完整后续转移；
- 因此它只能粗略识别 `ID`、`NUM` 和四则运算符，不能直接替代实验二完整 scanner。

## 输入文法格式

支持扩展 BNF 的 `|`：

```text
E -> E + T | T
T -> F * | F
F -> ( E ) | a
```

程序会自动增广文法，加入：

```text
E' -> E
```

## 输出内容

程序会输出：

- 增广文法；
- LR(0) 项目集规范族；
- 每个状态的闭包 `Closure(Ii)`；
- GOTO 状态转移；
- LR(0) 冲突检查结果；
- `lr0_items.dot` 状态图描述文件；
- 对 `test.src` 或实验二 scanner 输出 token 流的终结符归一化结果；

程序中不再直接输出实验一扩展 DFA 的报错原因分析，这部分可在实验报告中单独书写。

## LR(0) 文法判断思路

程序对每个项目集进行冲突检查：

- 如果一个项目集中同时存在“点号后面是终结符”的移进项目，以及“点号已经在产生式末尾”的归约项目，则存在移进-归约冲突；
- 如果一个项目集中同时存在两个及以上归约项目，则存在归约-归约冲突；
- 如果所有项目集都不存在上述两类冲突，则该文法可判定为 LR(0) 文法。

例如当前 `grammar.txt` 中的文法会在 `I4` 中出现：

```text
T -> F · *
T -> F ·
```

因此 `I4` 同时可以在 `*` 上移进，又可以按 `T -> F` 归约，存在移进-归约冲突，所以它不是严格 LR(0) 文法。

## 一个符合 LR(0) 的示例

本目录提供了 `grammar_lr0_ok.txt`：

```text
S -> a S | b
```

它可以识别 `b`、`ab`、`aab`、`aaab` 等串。运行：

```powershell
.\lr0.exe grammar_lr0_ok.txt
```

若输出“未发现移进-归约冲突或归约-归约冲突”，则说明该示例可视为 LR(0) 文法。

## 状态图

本目录提供两个状态图文件：

- `lr0_items.dot`：程序运行后自动生成，可用 Graphviz 渲染。
- `lr0_items.svg`：已整理好的状态图，可直接用浏览器打开或插入实验报告。

如果云服务器安装了 Graphviz，也可以运行：

```bash
dot -Tpng lr0_items.dot -o lr0_items.png
```

本地或服务器没有 Graphviz 时，直接使用 `lr0_items.svg` 即可。
