
# S3C2440 + K9F1208U0B NAND Flash 启动与读写代码

## 文件说明

- `s3c2440_k9f1208_nand_boot.c`：单文件版本，包含全部核心代码。
- `s3c2440_k9f1208_nand.h`：函数声明，便于拆分工程时引用。

## 代码覆盖内容

1. S3C2440 NAND 控制器初始化：`nand_init()`
2. Read1 读取主数据区：`nand_read_page_512()`
3. Read2 读取 spare 区：`nand_read_spare_16()`
4. 坏块标记检查：`nand_is_bad_block()`
5. 超过 4KB 代码搬运：`nand_copy_pages_to_sdram()`
6. 可选 Sequential Row Read1：`nand_read_block_seq_read1()`
7. Block Erase：`nand_erase_block()`
8. Page Program：`nand_program_page_512()`
9. 镜像写入 NAND：`nand_write_image()`
10. 加载到 SDRAM 并跳转：`bootloader_load_and_jump()`

## 启动流程

S3C2440 NAND boot 模式下，硬件自动把 NAND 前 4KB 拷贝到片内 SRAM/Steppingstone。
这段一级 BootLoader 初始化 SDRAM 和 NAND 控制器后，使用 Read1 Page Read
把 NAND 中后续代码读到 SDRAM，再跳转到 SDRAM 执行。

## 命令定位

- Read1 `00h/01h`：读取主数据区，适合读取程序代码。
- Read2 `50h`：读取 spare 区，适合读取坏块标记、ECC 等，不适合读取程序主体。
- Page Program `80h + 10h`：用于把镜像写入 NAND，属于存储/烧录阶段。
- Block Erase `60h + D0h`：写入前必须按块擦除。

## 工程注意事项

此代码为实验教学模板。实际工程还需要加入：
- SDRAM 初始化代码；
- 时钟初始化代码；
- 异常向量与启动汇编；
- ECC 校验；
- 完整坏块表管理；
- 与镜像烧录布局一致的逻辑到物理地址映射。
