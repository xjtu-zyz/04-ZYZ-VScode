#!/usr/bin/env bash
set -e
arm-linux-gcc -DS3C2440_TARGET -DTEST_LEVEL=3 -DBOARD_TEST_START_BLOCK=100 -c s3c2440_nand_three_level_tests.c -o nand_test.o
echo "nand_test.o generated. Link it with your S3C2440 startup/clock/SDRAM project."
