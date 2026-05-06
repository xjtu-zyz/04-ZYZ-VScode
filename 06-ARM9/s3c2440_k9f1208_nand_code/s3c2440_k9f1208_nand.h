
#ifndef S3C2440_K9F1208_NAND_H
#define S3C2440_K9F1208_NAND_H

typedef unsigned int  U32;
typedef unsigned char U8;

#define NAND_PAGE_SIZE          512U
#define NAND_SPARE_SIZE         16U
#define NAND_FULL_PAGE_SIZE     528U
#define NAND_PAGES_PER_BLOCK    32U
#define NAND_BLOCK_MAIN_SIZE    (NAND_PAGE_SIZE * NAND_PAGES_PER_BLOCK)

void nand_init(void);
void nand_read_page_512(U32 page, U8 *buf);
void nand_read_spare_16(U32 page, U8 *spare);
int  nand_is_bad_block(U32 block);
void nand_copy_pages_to_sdram(U32 nand_start_page, U8 *sdram_addr, U32 size);
int  nand_copy_image_skip_bad(U32 start_block, U8 *dst, U32 size);
void nand_read_block_seq_read1(U32 block, U8 *dst, U32 pages_to_read);
int  nand_erase_block(U32 block);
int  nand_program_page_512(U32 page, const U8 *buf);
int  nand_write_image(U32 start_block, const U8 *image, U32 size);
void bootloader_load_and_jump(void);
void bootloader_load_and_jump_skip_bad(void);

#endif
