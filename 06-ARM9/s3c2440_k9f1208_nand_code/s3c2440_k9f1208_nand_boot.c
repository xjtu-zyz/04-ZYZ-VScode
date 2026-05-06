
/*
 * s3c2440_k9f1208_nand_boot.c
 *
 * Target:
 *   Samsung S3C2440 + Samsung K9F1208U0B 64M x 8bit small-page NAND Flash
 *
 * Purpose:
 *   1. Initialize S3C2440 NAND controller.
 *   2. Read K9F1208U0B page main area and spare area.
 *   3. Check bad block marker.
 *   4. Copy code larger than 4KB from NAND Flash to SDRAM and jump to SDRAM.
 *   5. Provide Page Program and Block Erase functions for image storage/update.
 *
 * Notes:
 *   - K9F1208U0B page organization: 512B main + 16B spare.
 *   - One block contains 32 pages.
 *   - S3C2440 NAND boot hardware automatically copies the first 4KB from NAND
 *     to the internal SRAM/Steppingstone. The code below is intended to be used
 *     by that first-stage boot code after SDRAM has been initialized.
 *   - Read2 command 0x50 is for spare area access, not for reading the main
 *     program image continuously.
 */

typedef unsigned int  U32;
typedef unsigned char U8;

#define REG32(addr) (*(volatile U32 *)(addr))
#define REG8(addr)  (*(volatile U8  *)(addr))

/* S3C2440 NAND controller registers */
#define NFCONF      REG32(0x4E000000)
#define NFCONT      REG32(0x4E000004)
#define NFCMD REG8(0x4E000008)
#define NFADDR      REG8 (0x4E00000C)
#define NFDATA      REG8 (0x4E000010)
#define NFMECCD0    REG32(0x4E000014)
#define NFMECCD1    REG32(0x4E000018)
#define NFSECCD     REG32(0x4E00001C)
#define NFSTAT      REG32(0x4E000020)

/* K9F1208U0B geometry */
#define NAND_PAGE_SIZE          512U
#define NAND_SPARE_SIZE         16U
#define NAND_FULL_PAGE_SIZE     528U
#define NAND_PAGES_PER_BLOCK    32U
#define NAND_BLOCK_MAIN_SIZE    (NAND_PAGE_SIZE * NAND_PAGES_PER_BLOCK)

/* K9F1208U0B command set */
#define NAND_CMD_READ1_A        0x00U   /* main area A: 0~255 */
#define NAND_CMD_READ1_B        0x01U   /* main area B: 256~511 */
#define NAND_CMD_READ2          0x50U   /* spare area C: 512~527 */
#define NAND_CMD_READ_ID        0x90U
#define NAND_CMD_RESET          0xFFU
#define NAND_CMD_PROGRAM        0x80U
#define NAND_CMD_PROGRAM_END    0x10U
#define NAND_CMD_ERASE          0x60U
#define NAND_CMD_ERASE_END      0xD0U
#define NAND_CMD_STATUS         0x70U

/* Boot image layout example */
#define SDRAM_BASE              0x30000000U
#define SECOND_STAGE_PAGE       8U              /* page 0~7 = first 4KB */
#define SECOND_STAGE_SIZE       (128U * 1024U)  /* example: 128KB */

/* -------------------------------------------------------------------------- */
/* Low-level NAND control                                                     */
/* -------------------------------------------------------------------------- */

static void nand_select(void)
{
    NFCONT &= ~(1U << 1);     /* nFCE = 0, select NAND */
}

static void nand_deselect(void)
{
    NFCONT |= (1U << 1);      /* nFCE = 1, deselect NAND */
}

static void nand_wait_ready(void)
{
    while (!(NFSTAT & 0x01U)) {
        /* R/B is busy. Wait until ready. */
    }
}

static U8 nand_read_status(void)
{
    U8 status;
    NFCMD = (U8)NAND_CMD_STATUS;
    status = NFDATA;
    return status;
}

static int nand_status_ok(void)
{
    U8 status;

    do {
        status = nand_read_status();
    } while (!(status & (1U << 6)));    /* I/O6 = 1: Ready */

    return ((status & 0x01U) == 0U);    /* I/O0 = 0: Pass */
}

/*
 * Address cycles for K9F1208U0B small-page NAND:
 *   1st cycle: column A0~A7
 *   2nd cycle: row/page A9~A16
 *   3rd cycle: row/page A17~A24
 *   4th cycle: row/page A25, others low
 *
 * A8 is not sent as an address cycle. It is selected by command:
 *   00h selects first half of main area.
 *   01h selects second half of main area.
 *   50h selects spare area.
 */
static void nand_send_addr_4cycles(U32 page, U32 column)
{
    NFADDR = (U8)(column & 0xFFU);
    NFADDR = (U8)(page & 0xFFU);
    NFADDR = (U8)((page >> 8) & 0xFFU);
    NFADDR = (U8)((page >> 16) & 0x01U);
}

static void nand_send_row_addr_3cycles(U32 page)
{
    NFADDR = (U8)(page & 0xFFU);
    NFADDR = (U8)((page >> 8) & 0xFFU);
    NFADDR = (U8)((page >> 16) & 0x01U);
}

void nand_init(void)
{
    /*
     * Timing example for common S3C2440 boards.
     * Adjust TACLS/TWRPH0/TWRPH1 according to HCLK and the NAND data sheet.
     */
    const U32 TACLS  = 0;
    const U32 TWRPH0 = 3;
    const U32 TWRPH1 = 0;

    NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);

    /*
     * bit0: NAND controller enable
     * bit1: nFCE control, 1 means deselect
     * bit4: initialize ECC
     */
    NFCONT = (1U << 4) | (1U << 1) | (1U << 0);

    nand_select();
    NFCMD = (U8)NAND_CMD_RESET;
    nand_wait_ready();
    nand_deselect();
}

/* -------------------------------------------------------------------------- */
/* Read functions: used during boot to load code from NAND to SDRAM           */
/* -------------------------------------------------------------------------- */

void nand_read_page_512(U32 page, U8 *buf)
{
    U32 i;

    nand_select();

    NFCMD = (U8)NAND_CMD_READ1_A;       /* 00h: read main area from column 0 */
    nand_send_addr_4cycles(page, 0);
    nand_wait_ready();                  /* wait tR: cell -> page register */

    for (i = 0; i < NAND_PAGE_SIZE; i++) {
        buf[i] = NFDATA;
    }

    nand_deselect();
}

void nand_read_spare_16(U32 page, U8 *spare)
{
    U32 i;

    nand_select();

    NFCMD = (U8)NAND_CMD_READ2;         /* 50h: read spare area */
    nand_send_addr_4cycles(page, 0);
    nand_wait_ready();

    for (i = 0; i < NAND_SPARE_SIZE; i++) {
        spare[i] = NFDATA;
    }

    nand_deselect();
}

/*
 * Bad block marker for x8 K9F1208U0B:
 *   spare area 6th byte, i.e. full-page column 517.
 * Samsung recommends checking the 1st and 2nd page of each block.
 */
int nand_is_bad_block(U32 block)
{
    U8 spare[NAND_SPARE_SIZE];
    U32 page0 = block * NAND_PAGES_PER_BLOCK;
    U32 page1 = page0 + 1U;

    nand_read_spare_16(page0, spare);
    if (spare[5] != 0xFFU) {
        return 1;
    }

    nand_read_spare_16(page1, spare);
    if (spare[5] != 0xFFU) {
        return 1;
    }

    return 0;
}

/*
 * Simple page-by-page copy.
 * Suitable for first-stage bootloader: robust and easy to understand.
 */
void nand_copy_pages_to_sdram(U32 nand_start_page, U8 *sdram_addr, U32 size)
{
    U32 copied = 0;
    U32 page = nand_start_page;

    while (copied < size) {
        nand_read_page_512(page, sdram_addr + copied);
        page++;
        copied += NAND_PAGE_SIZE;
    }
}

/*
 * Copy image while skipping bad blocks.
 * This is more suitable for larger images. In a real bootloader, the same
 * logical-to-physical mapping used during programming must be used here.
 */
int nand_copy_image_skip_bad(U32 start_block, U8 *dst, U32 size)
{
    U32 copied = 0;
    U32 block = start_block;

    while (copied < size) {
        U32 i;

        if (nand_is_bad_block(block)) {
            block++;
            continue;
        }

        for (i = 0; i < NAND_PAGES_PER_BLOCK && copied < size; i++) {
            U32 page = block * NAND_PAGES_PER_BLOCK + i;
            nand_read_page_512(page, dst + copied);
            copied += NAND_PAGE_SIZE;
        }

        block++;
    }

    return 0;
}

/*
 * Optional optimization:
 * Sequential Row Read1 within one block.
 *
 * Important:
 *   - Not a replacement for Read2.
 *   - Do not cross block boundary using this function.
 *   - To advance from one page to the next, this function also reads and
 *     discards the 16-byte spare area after each 512-byte main area.
 */
void nand_read_block_seq_read1(U32 block, U8 *dst, U32 pages_to_read)
{
    U32 page = block * NAND_PAGES_PER_BLOCK;
    U32 p;
    volatile U8 dummy;

    if (pages_to_read > NAND_PAGES_PER_BLOCK) {
        pages_to_read = NAND_PAGES_PER_BLOCK;
    }

    nand_select();

    NFCMD = (U8)NAND_CMD_READ1_A;
    nand_send_addr_4cycles(page, 0);
    nand_wait_ready();

    for (p = 0; p < pages_to_read; p++) {
        U32 i;

        for (i = 0; i < NAND_PAGE_SIZE; i++) {
            dst[p * NAND_PAGE_SIZE + i] = NFDATA;
        }

        for (i = 0; i < NAND_SPARE_SIZE; i++) {
            dummy = NFDATA;
            (void)dummy;
        }

        if (p + 1U < pages_to_read) {
            nand_wait_ready();
        }
    }

    nand_deselect();
}

/* -------------------------------------------------------------------------- */
/* Program/erase functions: used to store/update images in NAND               */
/* -------------------------------------------------------------------------- */

int nand_erase_block(U32 block)
{
    U32 page = block * NAND_PAGES_PER_BLOCK;

    nand_select();

    NFCMD = (U8)NAND_CMD_ERASE;       /* 60h */
    nand_send_row_addr_3cycles(page); /* block erase uses row address only */
    NFCMD = (U8)NAND_CMD_ERASE_END;   /* D0h */

    nand_wait_ready();

    if (!nand_status_ok()) {
        nand_deselect();
        return -1;
    }

    nand_deselect();
    return 0;
}

int nand_program_page_512(U32 page, const U8 *buf)
{
    U32 i;

    nand_select();

    /*
     * Set pointer to main area A before program.
     * Then issue Serial Data Input command 80h.
     */
    NFCMD = (U8)NAND_CMD_READ1_A;
    NFCMD = (U8)NAND_CMD_PROGRAM;
    nand_send_addr_4cycles(page, 0);

    for (i = 0; i < NAND_PAGE_SIZE; i++) {
        NFDATA = buf[i];
    }

    NFCMD = (U8)NAND_CMD_PROGRAM_END; /* 10h: start internal program */
    nand_wait_ready();

    if (!nand_status_ok()) {
        nand_deselect();
        return -1;
    }

    nand_deselect();
    return 0;
}

static void fill_ff(U8 *buf, U32 len)
{
    U32 i;
    for (i = 0; i < len; i++) {
        buf[i] = 0xFFU;
    }
}

/*
 * Write image into NAND by Block Erase + Page Program.
 * This function is for a programmer/updater, not for every normal boot.
 */
int nand_write_image(U32 start_block, const U8 *image, U32 size)
{
    U32 written = 0;
    U32 block = start_block;
    U8 page_buf[NAND_PAGE_SIZE];

    while (written < size) {
        U32 i;

        if (nand_is_bad_block(block)) {
            block++;
            continue;
        }

        if (nand_erase_block(block) != 0) {
            block++;
            continue;
        }

        for (i = 0; i < NAND_PAGES_PER_BLOCK && written < size; i++) {
            U32 page = block * NAND_PAGES_PER_BLOCK + i;
            U32 remain = size - written;
            U32 copy_len = (remain > NAND_PAGE_SIZE) ? NAND_PAGE_SIZE : remain;
            U32 j;

            fill_ff(page_buf, NAND_PAGE_SIZE);

            for (j = 0; j < copy_len; j++) {
                page_buf[j] = image[written + j];
            }

            if (nand_program_page_512(page, page_buf) != 0) {
                /*
                 * A real system should mark this block bad or update the bad
                 * block table, then retry in another good block.
                 */
                return -1;
            }

            written += copy_len;
        }

        block++;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* Boot example                                                               */
/* -------------------------------------------------------------------------- */

/*
 * This function should be called by the first 4KB boot code after:
 *   1. Watchdog is disabled.
 *   2. Clock is configured.
 *   3. SDRAM controller is initialized.
 *   4. Stack is valid.
 */
void bootloader_load_and_jump(void)
{
    void (*entry)(void);

    nand_init();

    /*
     * Example:
     * BootLoader2 starts from page 8, because page 0~7 are the first 4KB.
     * Copy it to SDRAM and jump.
     */
    nand_copy_pages_to_sdram(SECOND_STAGE_PAGE,
                             (U8 *)SDRAM_BASE,
                             SECOND_STAGE_SIZE);

    entry = (void (*)(void))SDRAM_BASE;
    entry();
}

/*
 * Alternative boot entry if the image is stored from a block boundary and
 * bad blocks should be skipped. For example, store BootLoader2 from block 1.
 */
void bootloader_load_and_jump_skip_bad(void)
{
    void (*entry)(void);
    U32 start_block = 1U;

    nand_init();

    nand_copy_image_skip_bad(start_block,
                             (U8 *)SDRAM_BASE,
                             SECOND_STAGE_SIZE);

    entry = (void (*)(void))SDRAM_BASE;
    entry();
}

#if defined(_WIN32) && !defined(S3C2440_TARGET)

#include <stdio.h>

int main(void)
{
    printf("This program contains S3C2440 memory-mapped register access.\n");
    printf("It cannot directly run on Windows, because addresses such as ");
    printf("0x4E000004 are S3C2440 physical registers.\n");
    printf("Please compile with S3C2440_TARGET and run it on the ARM9 board.\n");

    return 0;
}

#else

int main(void)
{
    nand_init();

    /*
     * 正常启动：从 NAND 读取超过 4KB 的二级程序到 SDRAM，
     * 然后跳转执行。
     */
    bootloader_load_and_jump_skip_bad();

    while (1)
        ;
}

#endif