# Lab 2 Scanner

## Build

```powershell
g++ scanner.cpp -std=c++11 -Wall -Wextra -o scanner.exe
```

## Mode 1

Input one token count and then tokens separated by spaces.

```text
1
5
id if 485 841.6541 www
```

Output:

```text
ID
IF
NUM
FLOAT
ID
```

## Mode 2

Input one source statement and output token types.

```text
2
while(true) {int a=0;}
```

Output:

```text
WHILE
LPAR
ID
RPAR
LBR
ID
ID
ASG
NUM
SEMI
RBR
```

## Mode 3

Scan a file and output `(TYPE, lexeme)` pairs.

```powershell
.\scanner.exe test.c tokens.txt
```

or:

```text
3
test.c
```

Redirection is also supported:

```powershell
.\scanner.exe test.c > tokens.txt
```
