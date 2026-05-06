#!/usr/bin/env bash
set -e
gcc -std=c99 -Wall -Wextra -DTEST_LEVEL=1 s3c2440_nand_three_level_tests.c -o test_level1_mock
./test_level1_mock
