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
    end = end == -1 ? strlen(str) - 1 : end;
    uint64_t uv = 0;
    // 0 - positive; 1 - negative
    int signBit = 0;
    int state = 0;

    for (size_t i = start; i <= end; i++) {
        char ch = str[i];

        if (state == 0) {
            if (ch == ' ') {
                state = 0;
                continue;
            }
            // may be hex or dec
            if (ch == '0') {
                uv = 0;
                state = 1;
                continue;
            }
            if ('1' <= ch && ch <= '9') {
                state = 2;
                // cast ascii to number which it mapping
                uv = ch - '0';
                continue;
            }
            // may be negative
            if (ch == '-') {
                state = 3;
                signBit = 1;
                continue;
            }
            goto fail;
        }
        if (state == 1) {
            // dec
            if ('0' <= ch && ch <= '9') {
                state = 2;
                uv = uv * 10 + ch - '0';
                continue;
            }
            // hex
            if (ch == 'x') {
                state = 4;
                continue;
            }
            if (ch == ' ') {
                state = 6;
                continue;
            }
            goto fail;
        }
        if (state == 2) {
            if ('0' <= ch && ch <= '9') {
                state = 2;
                uint64_t pv = uv;
                uv = uv * 10 + ch - '0';
                // may be overflow
                if (pv > uv) {
                    elog("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                continue;
            }
            if (ch == ' ') {
                state = 6;
                continue;
            }
            goto fail;
        }
        if (state == 3) {
            if (ch == '0') {
                state = 1;
                continue;
            }
            if ('1' <= ch && ch <= '9') {
                state = 2;
                // cast ascii to number which it mapping
                uv = ch - '0';
                continue;
            }
            goto fail;
        }
        if (state == 4) {
            if ('0' <= ch && ch <= '9') {
                state = 5;
                uv = uv * 16 + ch - '0';
                continue;
            }
            if ('a' <= ch && ch <= 'f') {
                state = 5;
                uv = uv * 16 + ch - 'a' + 10;
                continue;
            }
            goto fail;
        }
        if (state == 5) {
            // hex
            if ('0' <= ch && ch <= '9') {
                state = 5;
                uint64_t pv = uv;
                uv = uv * 16 + ch - '0';
                // maybe overflow
                if (pv > uv) {
                    elog("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                continue;
            } else if ('a' <= ch && ch <= 'f') {
                state = 5;
                uint64_t pv = uv;
                uv = uv * 16 + ch - 'a' + 10;
                // maybe overflow
                if (pv > uv) {
                    elog("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                continue;
            }
            goto fail;
        }
        if (state == 6) {
            if (ch == ' ') {
                state = 6;
                continue;
            }
            goto fail;
        }
    }

    if (signBit == 0) {
        return uv;
    }

    if ((uv >> 63) == 1) {
        elog("(int64_t)%s: signed overflow: cannot convert\n", str);
        exit(0);
    }
    int64_t sv = -1 * (int64_t) uv;
    return *((uint64_t *) &sv);

    fail:
    elog("type converter: <%s> cannot be converted to integer\n", str);
    exit(0);
}


bool startsWith(const char *pre, const char *str) {
    size_t preLen = strlen(pre);
    size_t strLen = strlen(str);
    return strLen < preLen ? false : memcmp(pre, str, preLen) == 0;
}