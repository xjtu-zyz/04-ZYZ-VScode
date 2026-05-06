/*
 * s3c2440_nand_three_level_tests.c
 *
 * Three-level test code for Samsung S3C2440 + K9F1208U0B NAND Flash.
 *
 * Compile examples:
 *   Level 1, Windows/Linux host functional mock:
 *     gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=1 s3c2440_nand_three_level_tests.c -o test_level1_mock
 *
 *   Level 2, Windows/Linux host command-sequence mock:
 *     gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=2 s3c2440_nand_three_level_tests.c -o test_level2_trace
 *
 *   Level 3, S3C2440 target board:
 *     arm-linux-gcc -DS3C2440_TARGET -DTEST_LEVEL=3 -c s3c2440_nand_three_level_tests.c -o nand_test.o
 *     Link nand_test.o into your bare-metal S3C2440 project with startup, clock init and SDRAM init.
 *
 * Important:
 *   - Host levels do not touch real physical addresses, so they do not segfault on Windows.
 *   - Target level touches S3C2440 NAND registers and must run on the board.
 *   - Do not erase a NAND block that stores your bootloader/kernel/rootfs. Adjust BOARD_TEST_START_BLOCK.
 */

#ifndef TEST_LEVEL
#define TEST_LEVEL 1
#endif

#if (TEST_LEVEL == 1) || (TEST_LEVEL == 2)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

typedef unsigned int  U32;
typedef unsigned char U8;

/* K9F1208U0B geometry: 512-byte main area + 16-byte spare area, 32 pages/block. */
#define NAND_PAGE_SIZE          512U
#define NAND_SPARE_SIZE         16U
#define NAND_FULL_PAGE_SIZE     528U
#define NAND_PAGES_PER_BLOCK    32U
#define NAND_BLOCK_MAIN_SIZE    (NAND_PAGE_SIZE * NAND_PAGES_PER_BLOCK)

/* K9F1208U0B command set. */
#define NAND_CMD_READ1_A        0x00U   /* main area A: byte 0~255 */
#define NAND_CMD_READ1_B        0x01U   /* main area B: byte 256~511 */
#define NAND_CMD_READ2          0x50U   /* spare area C: byte 512~527 */
#define NAND_CMD_READ_ID        0x90U
#define NAND_CMD_RESET          0xFFU
#define NAND_CMD_PROGRAM        0x80U
#define NAND_CMD_PROGRAM_END    0x10U
#define NAND_CMD_ERASE          0x60U
#define NAND_CMD_ERASE_END      0xD0U
#define NAND_CMD_STATUS         0x70U

#define SDRAM_BASE              0x30000000U
#define SECOND_STAGE_PAGE       8U      /* page 0~7 is the first 4KB boot area */
#define SECOND_STAGE_SIZE       (128U * 1024U)

/* -------------------------------------------------------------------------- */
/* HAL interface. The driver below only calls these functions.                */
/* -------------------------------------------------------------------------- */

static void nand_hw_init(void);
static void nand_hw_select(void);
static void nand_hw_deselect(void);
static void nand_hw_write_cmd(U8 cmd);
static void nand_hw_write_addr(U8 addr);
static void nand_hw_write_data(U8 data);
static U8   nand_hw_read_data(void);
static void nand_hw_wait_ready(void);

/* -------------------------------------------------------------------------- */
/* Common NAND driver under test.                                             */
/* -------------------------------------------------------------------------- */

static void nand_send_addr_4cycles(U32 page, U32 column)
{
    /*
     * Small-page x8 NAND address cycles:
     *   1st: column A0~A7
     *   2nd: row/page A9~A16
     *   3rd: row/page A17~A24
     *   4th: row/page A25, upper bits low
     * A8 is selected by READ1_A/READ1_B/READ2 pointer command.
     */
    nand_hw_write_addr((U8)(column & 0xFFU));
    nand_hw_write_addr((U8)(page & 0xFFU));
    nand_hw_write_addr((U8)((page >> 8) & 0xFFU));
    nand_hw_write_addr((U8)((page >> 16) & 0x01U));
}

static void nand_send_row_addr_3cycles(U32 page)
{
    nand_hw_write_addr((U8)(page & 0xFFU));
    nand_hw_write_addr((U8)((page >> 8) & 0xFFU));
    nand_hw_write_addr((U8)((page >> 16) & 0x01U));
}

static U8 nand_read_status(void)
{
    nand_hw_write_cmd((U8)NAND_CMD_STATUS);
    return nand_hw_read_data();
}

static int nand_status_ok(void)
{
    U8 status;

    do {
        status = nand_read_status();
    } while ((status & (1U << 6)) == 0U);     /* I/O6 = 1 means Ready. */

    return ((status & 0x01U) == 0U);          /* I/O0 = 0 means Pass. */
}

void nand_init(void)
{
    nand_hw_init();
    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_RESET);
    nand_hw_wait_ready();
    nand_hw_deselect();
}

void nand_read_id(U8 id[4])
{
    U32 i;

    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_READ_ID);
    nand_hw_write_addr(0x00U);
    for (i = 0; i < 4U; i++) {
        id[i] = nand_hw_read_data();
    }
    nand_hw_deselect();
}

void nand_read_page_512(U32 page, U8 *buf)
{
    U32 i;

    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_READ1_A);
    nand_send_addr_4cycles(page, 0U);
    nand_hw_wait_ready();

    for (i = 0; i < NAND_PAGE_SIZE; i++) {
        buf[i] = nand_hw_read_data();
    }

    nand_hw_deselect();
}

void nand_read_spare_16(U32 page, U8 *spare)
{
    U32 i;

    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_READ2);
    nand_send_addr_4cycles(page, 0U);
    nand_hw_wait_ready();

    for (i = 0; i < NAND_SPARE_SIZE; i++) {
        spare[i] = nand_hw_read_data();
    }

    nand_hw_deselect();
}

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

int nand_erase_block(U32 block)
{
    U32 page = block * NAND_PAGES_PER_BLOCK;

    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_ERASE);
    nand_send_row_addr_3cycles(page);
    nand_hw_write_cmd((U8)NAND_CMD_ERASE_END);
    nand_hw_wait_ready();

    if (!nand_status_ok()) {
        nand_hw_deselect();
        return -1;
    }

    nand_hw_deselect();
    return 0;
}

int nand_program_page_512(U32 page, const U8 *buf)
{
    U32 i;

    nand_hw_select();

    /* Set pointer to main area A, then issue Serial Data Input command. */
    nand_hw_write_cmd((U8)NAND_CMD_READ1_A);
    nand_hw_write_cmd((U8)NAND_CMD_PROGRAM);
    nand_send_addr_4cycles(page, 0U);

    for (i = 0; i < NAND_PAGE_SIZE; i++) {
        nand_hw_write_data(buf[i]);
    }

    nand_hw_write_cmd((U8)NAND_CMD_PROGRAM_END);
    nand_hw_wait_ready();

    if (!nand_status_ok()) {
        nand_hw_deselect();
        return -1;
    }

    nand_hw_deselect();
    return 0;
}

static void fill_ff(U8 *buf, U32 len)
{
    U32 i;
    for (i = 0; i < len; i++) {
        buf[i] = 0xFFU;
    }
}

int nand_write_image(U32 start_block, const U8 *image, U32 size)
{
    U32 written = 0U;
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

        for (i = 0U; i < NAND_PAGES_PER_BLOCK && written < size; i++) {
            U32 page = block * NAND_PAGES_PER_BLOCK + i;
            U32 remain = size - written;
            U32 copy_len = (remain > NAND_PAGE_SIZE) ? NAND_PAGE_SIZE : remain;
            U32 j;

            fill_ff(page_buf, NAND_PAGE_SIZE);
            for (j = 0U; j < copy_len; j++) {
                page_buf[j] = image[written + j];
            }

            if (nand_program_page_512(page, page_buf) != 0) {
                return -1;
            }

            written += copy_len;
        }

        block++;
    }

    return 0;
}

void nand_copy_pages_to_sdram(U32 nand_start_page, U8 *sdram_addr, U32 size)
{
    U32 copied = 0U;
    U32 page = nand_start_page;

    while (copied < size) {
        nand_read_page_512(page, sdram_addr + copied);
        page++;
        copied += NAND_PAGE_SIZE;
    }
}

int nand_copy_image_skip_bad(U32 start_block, U8 *dst, U32 size)
{
    U32 copied = 0U;
    U32 block = start_block;

    while (copied < size) {
        U32 i;

        if (nand_is_bad_block(block)) {
            block++;
            continue;
        }

        for (i = 0U; i < NAND_PAGES_PER_BLOCK && copied < size; i++) {
            U32 page = block * NAND_PAGES_PER_BLOCK + i;
            nand_read_page_512(page, dst + copied);
            copied += NAND_PAGE_SIZE;
        }

        block++;
    }

    return 0;
}

void nand_read_block_seq_read1(U32 block, U8 *dst, U32 pages_to_read)
{
    U32 page = block * NAND_PAGES_PER_BLOCK;
    U32 p;
    volatile U8 dummy;

    if (pages_to_read > NAND_PAGES_PER_BLOCK) {
        pages_to_read = NAND_PAGES_PER_BLOCK;
    }

    nand_hw_select();
    nand_hw_write_cmd((U8)NAND_CMD_READ1_A);
    nand_send_addr_4cycles(page, 0U);
    nand_hw_wait_ready();

    for (p = 0U; p < pages_to_read; p++) {
        U32 i;

        for (i = 0U; i < NAND_PAGE_SIZE; i++) {
            dst[p * NAND_PAGE_SIZE + i] = nand_hw_read_data();
        }

        /* Read and discard spare bytes so the device can advance to next page. */
        for (i = 0U; i < NAND_SPARE_SIZE; i++) {
            dummy = nand_hw_read_data();
            (void)dummy;
        }

        if (p + 1U < pages_to_read) {
            nand_hw_wait_ready();
        }
    }

    nand_hw_deselect();
}

/* -------------------------------------------------------------------------- */
/* Mock HAL for level 1 and level 2.                                          */
/* -------------------------------------------------------------------------- */

#if (TEST_LEVEL == 1) || (TEST_LEVEL == 2)

#define MOCK_BLOCK_COUNT        128U
#define MOCK_TOTAL_PAGES        (MOCK_BLOCK_COUNT * NAND_PAGES_PER_BLOCK)

static U8 mock_nand[MOCK_BLOCK_COUNT][NAND_PAGES_PER_BLOCK][NAND_FULL_PAGE_SIZE];

#define EV_SELECT      1
#define EV_DESELECT    2
#define EV_CMD         3
#define EV_ADDR        4
#define EV_DATA_W      5
#define EV_DATA_R      6
#define EV_WAIT        7
#define EV_INIT        8

#define TRACE_MAX      4096U

typedef struct {
    int kind;
    U32 value;
} TraceEvent;

static TraceEvent g_trace[TRACE_MAX];
static U32 g_trace_count;

static U32 mock_cur_page;
static U32 mock_cur_col;
static U8  mock_cur_cmd;
static U8  mock_last_status;
static U8  mock_id[4] = {0xECU, 0x76U, 0xA5U, 0xC0U};
static U32 mock_id_index;

static int mock_addr_mode;       /* 0 none, 3 row only, 4 column+row, 1 ID address */
static int mock_addr_cycle;
static U32 mock_addr_column;
static U32 mock_addr_page;
static U8 mock_pending_cmd;

static void trace_add(int kind, U32 value)
{
    if (g_trace_count < TRACE_MAX) {
        g_trace[g_trace_count].kind = kind;
        g_trace[g_trace_count].value = value;
        g_trace_count++;
    }
}

// static void trace_clear(void)
// {
//     g_trace_count = 0U;
// }

// static const char *event_name(int kind)
// {
//     switch (kind) {
//     case EV_SELECT:   return "SELECT";
//     case EV_DESELECT: return "DESELECT";
//     case EV_CMD:      return "CMD";
//     case EV_ADDR:     return "ADDR";
//     case EV_DATA_W:   return "DATA_W";
//     case EV_DATA_R:   return "DATA_R";
//     case EV_WAIT:     return "WAIT";
//     case EV_INIT:     return "INIT";
//     default:          return "UNKNOWN";
//     }
// }

// static void trace_dump(void)
// {
//     U32 i;
//     for (i = 0U; i < g_trace_count; i++) {
//         if (g_trace[i].kind == EV_CMD || g_trace[i].kind == EV_ADDR) {
//             printf("%04u: %-8s 0x%02X\n", i, event_name(g_trace[i].kind), g_trace[i].value & 0xFFU);
//         } else {
//             printf("%04u: %-8s %u\n", i, event_name(g_trace[i].kind), g_trace[i].value);
//         }
//     }
// }

static void mock_reset_addr_parser(void)
{
    mock_addr_mode = 0;
    mock_addr_cycle = 0;
    mock_addr_column = 0U;
    mock_addr_page = 0U;
}

static void mock_set_all_ff(void)
{
    memset(mock_nand, 0xFF, sizeof(mock_nand));
}

static void mock_mark_bad_block(U32 block)
{
    if (block < MOCK_BLOCK_COUNT) {
        mock_nand[block][0][NAND_PAGE_SIZE + 5U] = 0x00U;
        mock_nand[block][1][NAND_PAGE_SIZE + 5U] = 0x00U;
    }
}

static int mock_check_range(U32 page)
{
    return (page < MOCK_TOTAL_PAGES);
}

static void mock_finish_addr_if_needed(void)
{
    if (mock_addr_mode == 4 && mock_addr_cycle == 4) {
        mock_cur_page = mock_addr_page;
        if (mock_pending_cmd == NAND_CMD_READ2) {
            mock_cur_col = NAND_PAGE_SIZE + (mock_addr_column & 0x0FU);
        } else {
            mock_cur_col = mock_addr_column;
        }
        mock_reset_addr_parser();
    } else if (mock_addr_mode == 3 && mock_addr_cycle == 3) {
        mock_cur_page = mock_addr_page;
        mock_cur_col = 0U;
        mock_reset_addr_parser();
    } else if (mock_addr_mode == 1 && mock_addr_cycle == 1) {
        mock_id_index = 0U;
        mock_reset_addr_parser();
    }
}

static void nand_hw_init(void)
{
    mock_set_all_ff();
    mock_cur_page = 0U;
    mock_cur_col = 0U;
    mock_cur_cmd = 0U;
    mock_last_status = 0x40U;   /* Ready + Pass. */
    mock_id_index = 0U;
    mock_reset_addr_parser();
    trace_add(EV_INIT, 0U);
}

static void nand_hw_select(void)
{
    trace_add(EV_SELECT, 0U);
}

static void nand_hw_deselect(void)
{
    trace_add(EV_DESELECT, 0U);
}

static void nand_hw_write_cmd(U8 cmd)
{
    U32 block;

    mock_cur_cmd = cmd;
    trace_add(EV_CMD, cmd);

    if (cmd == NAND_CMD_READ1_A || cmd == NAND_CMD_READ1_B || cmd == NAND_CMD_READ2 || cmd == NAND_CMD_PROGRAM) {
        mock_pending_cmd = cmd;
        mock_addr_mode = 4;
        mock_addr_cycle = 0;
        mock_addr_column = 0U;
        mock_addr_page = 0U;
    } else if (cmd == NAND_CMD_ERASE) {
        mock_pending_cmd = cmd;
        mock_addr_mode = 3;
        mock_addr_cycle = 0;
        mock_addr_page = 0U;
        mock_addr_column = 0U;
    } else if (cmd == NAND_CMD_READ_ID) {
        mock_pending_cmd = cmd;
        mock_addr_mode = 1;
        mock_addr_cycle = 0;
        mock_id_index = 0U;
    } else if (cmd == NAND_CMD_ERASE_END) {
        block = mock_cur_page / NAND_PAGES_PER_BLOCK;
        if (block < MOCK_BLOCK_COUNT) {
            memset(mock_nand[block], 0xFF, sizeof(mock_nand[block]));
            mock_last_status = 0x40U;
        } else {
            mock_last_status = 0x41U;
        }
    } else if (cmd == NAND_CMD_PROGRAM_END) {
        mock_last_status = 0x40U;
    } else if (cmd == NAND_CMD_RESET) {
        mock_last_status = 0x40U;
        mock_reset_addr_parser();
    } else {
        /* STATUS and other commands do not need parser state here. */
    }
}

static void nand_hw_write_addr(U8 addr)
{
    trace_add(EV_ADDR, addr);

    if (mock_addr_mode == 4) {
        if (mock_addr_cycle == 0) {
            mock_addr_column = addr;
        } else if (mock_addr_cycle == 1) {
            mock_addr_page |= (U32)addr;
        } else if (mock_addr_cycle == 2) {
            mock_addr_page |= ((U32)addr << 8);
        } else if (mock_addr_cycle == 3) {
            mock_addr_page |= ((U32)(addr & 0x01U) << 16);
        }
        mock_addr_cycle++;
    } else if (mock_addr_mode == 3) {
        if (mock_addr_cycle == 0) {
            mock_addr_page |= (U32)addr;
        } else if (mock_addr_cycle == 1) {
            mock_addr_page |= ((U32)addr << 8);
        } else if (mock_addr_cycle == 2) {
            mock_addr_page |= ((U32)(addr & 0x01U) << 16);
        }
        mock_addr_cycle++;
    } else if (mock_addr_mode == 1) {
        (void)addr;
        mock_addr_cycle++;
    }

    mock_finish_addr_if_needed();
}

static void nand_hw_write_data(U8 data)
{
    U32 block = mock_cur_page / NAND_PAGES_PER_BLOCK;
    U32 page_in_block = mock_cur_page % NAND_PAGES_PER_BLOCK;

    trace_add(EV_DATA_W, data);

    if (mock_check_range(mock_cur_page) && mock_cur_col < NAND_FULL_PAGE_SIZE) {
        /* NAND programming changes 1 to 0; it cannot change 0 back to 1 without erase. */
        mock_nand[block][page_in_block][mock_cur_col] &= data;
        mock_cur_col++;
    } else {
        mock_last_status = 0x41U;
    }
}

static U8 nand_hw_read_data(void)
{
    U8 value = 0xFFU;
    U32 block = mock_cur_page / NAND_PAGES_PER_BLOCK;
    U32 page_in_block = mock_cur_page % NAND_PAGES_PER_BLOCK;

    if (mock_cur_cmd == NAND_CMD_STATUS) {
        value = mock_last_status;
    } else if (mock_cur_cmd == NAND_CMD_READ_ID) {
        value = (mock_id_index < 4U) ? mock_id[mock_id_index++] : 0xFFU;
    } else {
        /* Sequential Row Read1: after reading the last column of a page,
         * the next read can advance to the next page within the same block. */
        if (mock_cur_col >= NAND_FULL_PAGE_SIZE) {
            mock_cur_page++;
            mock_cur_col = 0U;
            block = mock_cur_page / NAND_PAGES_PER_BLOCK;
            page_in_block = mock_cur_page % NAND_PAGES_PER_BLOCK;
        }

        if (mock_check_range(mock_cur_page) && mock_cur_col < NAND_FULL_PAGE_SIZE) {
            value = mock_nand[block][page_in_block][mock_cur_col];
            mock_cur_col++;
        }
    }

    trace_add(EV_DATA_R, value);
    return value;
}

static void nand_hw_wait_ready(void)
{
    trace_add(EV_WAIT, 0U);
}

#endif /* host mock HAL */

/* -------------------------------------------------------------------------- */
/* S3C2440 real HAL for level 3.                                              */
/* -------------------------------------------------------------------------- */

#if (TEST_LEVEL == 3) && defined(S3C2440_TARGET)

#define REG32(addr) (*(volatile U32 *)(addr))
#define REG8(addr)  (*(volatile U8  *)(addr))

#define NFCONF      REG32(0x4E000000)
#define NFCONT      REG32(0x4E000004)
#define NFCMD       REG8 (0x4E000008)
#define NFADDR      REG8 (0x4E00000C)
#define NFDATA      REG8 (0x4E000010)
#define NFSTAT      REG32(0x4E000020)

static void nand_hw_init(void)
{
    const U32 TACLS  = 0U;
    const U32 TWRPH0 = 3U;
    const U32 TWRPH1 = 0U;

    NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);
    NFCONT = (1U << 4) | (1U << 1) | (1U << 0);  /* Init ECC, deselect, enable controller. */
}

static void nand_hw_select(void)
{
    NFCONT &= ~(1U << 1);
}

static void nand_hw_deselect(void)
{
    NFCONT |= (1U << 1);
}

static void nand_hw_write_cmd(U8 cmd)
{
    NFCMD = cmd;
}

static void nand_hw_write_addr(U8 addr)
{
    NFADDR = addr;
}

static void nand_hw_write_data(U8 data)
{
    NFDATA = data;
}

static U8 nand_hw_read_data(void)
{
    return NFDATA;
}

static void nand_hw_wait_ready(void)
{
    while ((NFSTAT & 0x01U) == 0U) {
        /* Wait until R/B is Ready. */
    }
}

#endif /* real target HAL */

#if (TEST_LEVEL == 3) && !defined(S3C2440_TARGET)
#error "TEST_LEVEL=3 must be compiled with -DS3C2440_TARGET and run on S3C2440 board."
#endif

/* -------------------------------------------------------------------------- */
/* Level 1: host functional mock tests.                                       */
/* -------------------------------------------------------------------------- */

#if TEST_LEVEL == 1

#define TEST_IMAGE_SIZE     (40U * 1024U)   /* Larger than 4KB. */

static U8 test_image[TEST_IMAGE_SIZE];
static U8 read_back[TEST_IMAGE_SIZE];
static U8 seq_buf[NAND_PAGE_SIZE * 4U];

static int g_failures = 0;

#define CHECK_TRUE(cond, msg) do {                    \
    if (cond) {                                       \
        printf("[PASS] %s\n", msg);                  \
    } else {                                          \
        printf("[FAIL] %s\n", msg);                  \
        g_failures++;                                 \
    }                                                 \
} while (0)

static void make_pattern(U8 *buf, U32 len, U8 seed)
{
    U32 i;
    for (i = 0U; i < len; i++) {
        buf[i] = (U8)((i * 13U + seed) & 0xFFU);
    }
}

static int test_read_id(void)
{
    U8 id[4];
    nand_init();
    nand_read_id(id);
    return (id[0] == 0xECU && id[1] == 0x76U);
}

static int test_write_read_image_no_bad_block(void)
{
    nand_init();
    make_pattern(test_image, TEST_IMAGE_SIZE, 0x23U);
    memset(read_back, 0, sizeof(read_back));

    if (nand_write_image(1U, test_image, TEST_IMAGE_SIZE) != 0) {
        return 0;
    }

    if (nand_copy_image_skip_bad(1U, read_back, TEST_IMAGE_SIZE) != 0) {
        return 0;
    }

    return (memcmp(test_image, read_back, TEST_IMAGE_SIZE) == 0);
}

static int test_bad_block_skip(void)
{
    nand_init();
    make_pattern(test_image, TEST_IMAGE_SIZE, 0x5AU);
    memset(read_back, 0, sizeof(read_back));

    mock_mark_bad_block(1U);

    if (!nand_is_bad_block(1U)) {
        return 0;
    }

    if (nand_write_image(1U, test_image, TEST_IMAGE_SIZE) != 0) {
        return 0;
    }

    if (nand_copy_image_skip_bad(1U, read_back, TEST_IMAGE_SIZE) != 0) {
        return 0;
    }

    return (memcmp(test_image, read_back, TEST_IMAGE_SIZE) == 0);
}

static int test_read2_spare_bad_marker(void)
{
    U8 spare[NAND_SPARE_SIZE];
    U32 block = 6U;
    U32 page = block * NAND_PAGES_PER_BLOCK;

    nand_init();
    mock_mark_bad_block(block);
    nand_read_spare_16(page, spare);

    return (spare[5] == 0x00U && nand_is_bad_block(block));
}

static int test_sequential_row_read1_mock(void)
{
    U32 block = 10U;
    U32 p, i;
    U8 page_buf[NAND_PAGE_SIZE];

    nand_init();

    for (p = 0U; p < 4U; p++) {
        for (i = 0U; i < NAND_PAGE_SIZE; i++) {
            page_buf[i] = (U8)((p * 17U + i) & 0xFFU);
        }
        if (nand_program_page_512(block * NAND_PAGES_PER_BLOCK + p, page_buf) != 0) {
            return 0;
        }
    }

    memset(seq_buf, 0, sizeof(seq_buf));
    nand_read_block_seq_read1(block, seq_buf, 4U);

    for (p = 0U; p < 4U; p++) {
        for (i = 0U; i < NAND_PAGE_SIZE; i++) {
            U8 expected = (U8)((p * 17U + i) & 0xFFU);
            if (seq_buf[p * NAND_PAGE_SIZE + i] != expected) {
                return 0;
            }
        }
    }

    return 1;
}

int main(void)
{
    printf("S3C2440 NAND level 1 host functional mock tests\n");
    CHECK_TRUE(test_read_id(), "read ID mock returns Samsung/K9F1208-like ID");
    CHECK_TRUE(test_write_read_image_no_bad_block(), "write image >4KB, read back and compare");
    CHECK_TRUE(test_bad_block_skip(), "bad block marker is detected and skipped");
    CHECK_TRUE(test_read2_spare_bad_marker(), "Read2 reads spare area and bad-block marker");
    CHECK_TRUE(test_sequential_row_read1_mock(), "Sequential Row Read1 mock reads pages within one block");

    if (g_failures == 0) {
        printf("\nALL LEVEL 1 TESTS PASSED\n");
        return 0;
    }

    printf("\nLEVEL 1 TESTS FAILED: %d\n", g_failures);
    return 1;
}

#endif /* TEST_LEVEL == 1 */

/* -------------------------------------------------------------------------- */
/* Level 2: host command-sequence tests.                                      */
/* -------------------------------------------------------------------------- */

#if TEST_LEVEL == 2

static int g_failures = 0;

static int expect_event(U32 index, int kind, U32 value, const char *label)
{
    if (index >= g_trace_count) {
        printf("[FAIL] %s: missing event at index %u\n", label, index);
        g_failures++;
        return 0;
    }

    if (g_trace[index].kind != kind || g_trace[index].value != value) {
        printf("[FAIL] %s: event[%u] expected %s 0x%X, got %s 0x%X\n",
               label,
               index,
               event_name(kind),
               value,
               event_name(g_trace[index].kind),
               g_trace[index].value);
        g_failures++;
        return 0;
    }

    return 1;
}

static U32 count_event_kind(int kind)
{
    U32 i;
    U32 count = 0U;
    for (i = 0U; i < g_trace_count; i++) {
        if (g_trace[i].kind == kind) {
            count++;
        }
    }
    return count;
}

static void pass_if(int cond, const char *msg)
{
    if (cond) {
        printf("[PASS] %s\n", msg);
    } else {
        printf("[FAIL] %s\n", msg);
        g_failures++;
    }
}

static void test_trace_page_read(void)
{
    U8 buf[NAND_PAGE_SIZE];
    U32 page = 0x1234U;

    nand_init();
    trace_clear();
    nand_read_page_512(page, buf);

    expect_event(0U, EV_SELECT, 0U, "page read select");
    expect_event(1U, EV_CMD, NAND_CMD_READ1_A, "page read command 00h");
    expect_event(2U, EV_ADDR, 0x00U, "page read column addr");
    expect_event(3U, EV_ADDR, 0x34U, "page read row addr low");
    expect_event(4U, EV_ADDR, 0x12U, "page read row addr mid");
    expect_event(5U, EV_ADDR, 0x00U, "page read row addr high");
    expect_event(6U, EV_WAIT, 0U, "page read wait ready");
    pass_if(count_event_kind(EV_DATA_R) == NAND_PAGE_SIZE, "page read has exactly 512 data reads");
    pass_if(g_trace[g_trace_count - 1U].kind == EV_DESELECT, "page read ends with deselect");
}

static void test_trace_spare_read2(void)
{
    U8 spare[NAND_SPARE_SIZE];
    U32 page = 0x0007U;

    nand_init();
    trace_clear();
    nand_read_spare_16(page, spare);

    expect_event(0U, EV_SELECT, 0U, "spare read select");
    expect_event(1U, EV_CMD, NAND_CMD_READ2, "spare read command 50h");
    expect_event(2U, EV_ADDR, 0x00U, "spare read spare offset");
    expect_event(3U, EV_ADDR, 0x07U, "spare read row low");
    expect_event(4U, EV_ADDR, 0x00U, "spare read row mid");
    expect_event(5U, EV_ADDR, 0x00U, "spare read row high");
    expect_event(6U, EV_WAIT, 0U, "spare read wait ready");
    pass_if(count_event_kind(EV_DATA_R) == NAND_SPARE_SIZE, "spare read has exactly 16 data reads");
    pass_if(g_trace[g_trace_count - 1U].kind == EV_DESELECT, "spare read ends with deselect");
}

static void test_trace_page_program(void)
{
    U8 buf[NAND_PAGE_SIZE];
    U32 i;
    U32 page = 0x0021U;

    for (i = 0U; i < NAND_PAGE_SIZE; i++) {
        buf[i] = (U8)i;
    }

    nand_init();
    trace_clear();
    (void)nand_program_page_512(page, buf);

    expect_event(0U, EV_SELECT, 0U, "program select");
    expect_event(1U, EV_CMD, NAND_CMD_READ1_A, "program pointer 00h");
    expect_event(2U, EV_CMD, NAND_CMD_PROGRAM, "program serial input 80h");
    expect_event(3U, EV_ADDR, 0x00U, "program column addr");
    expect_event(4U, EV_ADDR, 0x21U, "program row low");
    expect_event(5U, EV_ADDR, 0x00U, "program row mid");
    expect_event(6U, EV_ADDR, 0x00U, "program row high");
    pass_if(count_event_kind(EV_DATA_W) == NAND_PAGE_SIZE, "program has exactly 512 data writes");
    pass_if(g_trace[g_trace_count - 4U].kind == EV_WAIT, "program waits ready after 10h");
    pass_if(g_trace[g_trace_count - 3U].kind == EV_CMD && g_trace[g_trace_count - 3U].value == NAND_CMD_STATUS,
            "program reads status with 70h");
    pass_if(g_trace[g_trace_count - 2U].kind == EV_DATA_R, "program reads one status byte");
    pass_if(g_trace[g_trace_count - 1U].kind == EV_DESELECT, "program ends with deselect");
}

static void test_trace_block_erase(void)
{
    U32 block = 3U;
    U32 page = block * NAND_PAGES_PER_BLOCK;

    nand_init();
    trace_clear();
    (void)nand_erase_block(block);

    expect_event(0U, EV_SELECT, 0U, "erase select");
    expect_event(1U, EV_CMD, NAND_CMD_ERASE, "erase setup 60h");
    expect_event(2U, EV_ADDR, (page & 0xFFU), "erase row low");
    expect_event(3U, EV_ADDR, ((page >> 8) & 0xFFU), "erase row mid");
    expect_event(4U, EV_ADDR, ((page >> 16) & 0x01U), "erase row high");
    expect_event(5U, EV_CMD, NAND_CMD_ERASE_END, "erase confirm D0h");
    expect_event(6U, EV_WAIT, 0U, "erase wait ready");
    expect_event(7U, EV_CMD, NAND_CMD_STATUS, "erase read status 70h");
    pass_if(g_trace[g_trace_count - 2U].kind == EV_DATA_R, "erase reads one status byte");
    pass_if(g_trace[g_trace_count - 1U].kind == EV_DESELECT, "erase ends with deselect");
}

int main(void)
{
    printf("S3C2440 NAND level 2 command-sequence mock tests\n");

    test_trace_page_read();
    test_trace_spare_read2();
    test_trace_page_program();
    test_trace_block_erase();

    if (g_failures == 0) {
        printf("\nALL LEVEL 2 TESTS PASSED\n");
        return 0;
    }

    printf("\nLEVEL 2 TESTS FAILED: %d\n", g_failures);
    printf("Last trace dump for debugging:\n");
    trace_dump();
    return 1;
}

#endif /* TEST_LEVEL == 2 */

/* -------------------------------------------------------------------------- */
/* Level 3: real S3C2440 board test.                                          */
/* -------------------------------------------------------------------------- */

#if TEST_LEVEL == 3

/* Change this to a safe empty block on your board. Do not use bootloader blocks. */
#ifndef BOARD_TEST_START_BLOCK
#define BOARD_TEST_START_BLOCK 100U
#endif

#ifndef BOARD_TEST_BLOCK_SEARCH_LIMIT
#define BOARD_TEST_BLOCK_SEARCH_LIMIT 32U
#endif

#ifndef ENABLE_SDRAM_COPY_TEST
#define ENABLE_SDRAM_COPY_TEST 0
#endif

typedef struct {
    volatile U32 magic;
    volatile U32 step;
    volatile U32 selected_block;
    volatile U32 selected_page;
    volatile U32 erase_ok;
    volatile U32 program_ok;
    volatile U32 read_compare_ok;
    volatile U32 copy_to_sdram_ok;
    volatile U32 error_count;
    volatile U8  id[4];
    volatile U8  read_back_head[16];
} BoardTestResult;

volatile BoardTestResult g_nand_board_result;
static U8 board_write_buf[NAND_PAGE_SIZE];
static U8 board_read_buf[NAND_PAGE_SIZE];

static void board_fill_pattern(void)
{
    U32 i;
    for (i = 0U; i < NAND_PAGE_SIZE; i++) {
        board_write_buf[i] = (U8)((i * 7U + 0x31U) & 0xFFU);
        board_read_buf[i] = 0U;
    }
}

static int board_compare_page(void)
{
    U32 i;
    U32 errors = 0U;

    for (i = 0U; i < NAND_PAGE_SIZE; i++) {
        if (board_write_buf[i] != board_read_buf[i]) {
            errors++;
        }
    }

    g_nand_board_result.error_count = errors;
    for (i = 0U; i < 16U; i++) {
        g_nand_board_result.read_back_head[i] = board_read_buf[i];
    }

    return (errors == 0U);
}

static U32 board_find_good_block(U32 start_block, U32 limit)
{
    U32 block;
    for (block = start_block; block < start_block + limit; block++) {
        if (!nand_is_bad_block(block)) {
            return block;
        }
    }
    return 0xFFFFFFFFU;
}

int main(void)
{
    U32 block;
    U32 page;
    U8 id[4];

    g_nand_board_result.magic = 0x2440CAFEU;
    g_nand_board_result.step = 1U;

    board_fill_pattern();

    nand_init();
    g_nand_board_result.step = 2U;

    nand_read_id(id);
    g_nand_board_result.id[0] = id[0];
    g_nand_board_result.id[1] = id[1];
    g_nand_board_result.id[2] = id[2];
    g_nand_board_result.id[3] = id[3];
    g_nand_board_result.step = 3U;

    block = board_find_good_block(BOARD_TEST_START_BLOCK, BOARD_TEST_BLOCK_SEARCH_LIMIT);
    g_nand_board_result.selected_block = block;

    if (block == 0xFFFFFFFFU) {
        g_nand_board_result.step = 0xE001U;
        while (1) { }
    }

    page = block * NAND_PAGES_PER_BLOCK;
    g_nand_board_result.selected_page = page;

    if (nand_erase_block(block) == 0) {
        g_nand_board_result.erase_ok = 1U;
    } else {
        g_nand_board_result.step = 0xE002U;
        while (1) { }
    }

    g_nand_board_result.step = 4U;

    if (nand_program_page_512(page, board_write_buf) == 0) {
        g_nand_board_result.program_ok = 1U;
    } else {
        g_nand_board_result.step = 0xE003U;
        while (1) { }
    }

    g_nand_board_result.step = 5U;

    nand_read_page_512(page, board_read_buf);
    if (board_compare_page()) {
        g_nand_board_result.read_compare_ok = 1U;
    } else {
        g_nand_board_result.step = 0xE004U;
        while (1) { }
    }

#if ENABLE_SDRAM_COPY_TEST
    /* Only enable this when SDRAM has been initialized and 0x30000000 is safe to overwrite. */
    nand_copy_image_skip_bad(block, (U8 *)SDRAM_BASE, NAND_PAGE_SIZE);
    g_nand_board_result.copy_to_sdram_ok = 1U;
#endif

    g_nand_board_result.step = 0x900DU;

    while (1) {
        /* Inspect g_nand_board_result in debugger. */
    }

    return 0;
}

#endif /* TEST_LEVEL == 3 */
