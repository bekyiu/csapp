//
// Created by bekyiu on 2021/7/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../header/common.h"

// covert string to int64_t
uint64_t str2uint(const char *str) {
    return str2uintRange(str, 0, -1);
}

uint64_t str2uintRange(const char *str, int start, int end) {
    return 0;
}