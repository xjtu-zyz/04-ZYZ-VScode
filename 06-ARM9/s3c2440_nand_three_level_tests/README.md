# S3C2440 + K9F1208U0B NAND Flash 三级测试代码

本代码用于验证 NAND Flash 驱动的三类问题：

1. Level 1：Windows/Linux 主机 mock 功能测试
   - 不访问真实物理寄存器。
   - 验证镜像写入、页读取、Read2 读取 spare、坏块跳过、超过 4KB 数据搬运、Sequential Row Read1 的软件逻辑。

2. Level 2：Windows/Linux 主机命令序列测试
   - 不访问真实物理寄存器。
   - 记录 NAND 驱动发出的 SELECT、CMD、ADDR、DATA、WAIT 事件。
   - 检查 Page Read、Read2、Page Program、Block Erase 是否符合 K9F1208U0B 命令/地址/数据流程。

3. Level 3：S3C2440 开发板实测
   - 会访问真实 S3C2440 NAND 控制器寄存器。
   - 必须在开发板或目标环境运行，不能在 Windows 用户态直接运行。
   - 测试 NAND ID、坏块查找、Block Erase、Page Program、Page Read、数据比较。
   - 结果保存在全局变量 g_nand_board_result，可用调试器查看。

## 编译命令

### Level 1：主机 mock 功能测试

Linux / MinGW / MSYS2：

```bash
gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=1 s3c2440_nand_three_level_tests.c -o test_level1_mock
./test_level1_mock
```

Windows PowerShell + MinGW：

```powershell
gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=1 s3c2440_nand_three_level_tests.c -o test_level1_mock.exe
.\test_level1_mock.exe
```

### Level 2：命令序列测试

```bash
gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=2 s3c2440_nand_three_level_tests.c -o test_level2_trace
./test_level2_trace
```

### Level 3：S3C2440 开发板测试

该文件需要加入你已有的 S3C2440 裸机工程，确保 startup、时钟初始化、SDRAM 初始化、栈初始化已经完成。

示例：

```bash
arm-linux-gcc -DS3C2440_TARGET -DTEST_LEVEL=3 -c s3c2440_nand_three_level_tests.c -o nand_test.o
```

如果要指定测试块：

```bash
arm-linux-gcc -DS3C2440_TARGET -DTEST_LEVEL=3 -DBOARD_TEST_START_BLOCK=100 -c s3c2440_nand_three_level_tests.c -o nand_test.o
```

注意：BOARD_TEST_START_BLOCK 必须选择安全空闲块，不能选择存放 BootLoader、Kernel、RootFS 的块。

## 目标板调试观察变量

Level 3 运行后，程序停在 while(1)，可在调试器中查看：

```c
g_nand_board_result
```

字段说明：

- magic = 0x2440CAFE 表示测试代码已经运行；
- step = 0x900D 表示测试完成；
- id[0..3] 为 NAND ID；
- selected_block 为实际选择测试的 NAND block；
- erase_ok/program_ok/read_compare_ok 为擦除、写入、读回比较结果；
- error_count 为读回比较错误字节数；
- read_back_head[16] 为读回数据前 16 字节。

## 重要提醒

- Level 1/2 只能证明软件逻辑和命令序列合理，不能证明真实硬件时序、电气连接、NFCONF 参数一定正确。
- Level 3 才能验证真实开发板寄存器访问、R/B 状态、nFCE/nFWE/nFRE/CLE/ALE 等信号连接和基本读写。
- 不要在开发板上随意擦除 block 0 或已存放系统镜像的 block。
